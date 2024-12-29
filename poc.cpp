#pragma leco app
#pragma leco add_shader "poc.vert"
#pragma leco add_shader "poc.frag"

import casein;
import dotz;
import rng;
import vee;
import voo;

static struct upc {
  dotz::vec2 aspect;
  dotz::vec2 displ {};
  float scale = 6;
} g_pc;

static constexpr const dotz::ivec2 grid_size { 16 };

static void load_grid(voo::h2l_buffer * buf) {
  voo::mapmem m { buf->host_memory() };
  auto ptr = static_cast<unsigned *>(*m);
  auto pp = ptr;
  for (auto y = 0; y < grid_size.y; y++) {
    for (auto x = 0; x < grid_size.x; x++, ptr++) {
      *ptr = (x + y) % 2;
    }
  }
  for (auto n = 0; n < 100; n++) {
    unsigned x = rng::rand(grid_size.x);
    unsigned y = rng::rand(grid_size.y);
    pp[y * grid_size.x + x] ^= 1;
  }
}

static void translate() {
  auto d = casein::mouse_rel / casein::window_size;
  g_pc.displ = g_pc.displ - d * 100.0;
}

struct init : public voo::casein_thread {
  init() : casein_thread {} {
    using namespace casein;
    handle(MOUSE_MOVE, M_WHEEL, translate);
  }

  void run() {
    main_loop("poc-voo", [&](auto & dq, auto & sw) {
      auto dsl = vee::create_descriptor_set_layout({
        vee::dsl_fragment_storage(),
      });
      vee::pipeline_layout pl = vee::create_pipeline_layout({ *dsl }, {
        vee::vert_frag_push_constant_range<upc>()
      });
      voo::one_quad_render oqr { "poc", &dq, *pl };

      constexpr const unsigned sz = grid_size.x * grid_size.y * sizeof(unsigned);
      auto buf = voo::updater { dq.queue(), &load_grid, dq, sz };
      buf.run_once();

      auto dpool = vee::create_descriptor_pool(1, {
        vee::storage_buffer(1),
      });
      auto dset = vee::allocate_descriptor_set(*dpool, *dsl);
      vee::update_descriptor_set_with_storage(dset, 0, buf.data().local_buffer());

      if (sw.aspect() > 1.0) {
        g_pc.aspect = { sw.aspect(), 1.0f };
      } else {
        g_pc.aspect = { 1.0f, 1.0f / sw.aspect() };
      }
      auto min = (g_pc.aspect - 1.0) * g_pc.scale / 2.0;
      auto max = grid_size - min - g_pc.scale;
      g_pc.displ = (max + min) / 2.0;

      ots_loop(dq, sw, [&](auto cb) {
        auto nxt = g_pc.displ;
        if (nxt.x > max.x) nxt.x -= (nxt.x - max.x) * 0.3;
        if (nxt.y > max.y) nxt.y -= (nxt.y - max.y) * 0.3;
        if (nxt.x < min.x) nxt.x -= (nxt.x - min.x) * 0.3;
        if (nxt.y < min.y) nxt.y -= (nxt.y - min.y) * 0.3;
        g_pc.displ = nxt;

        oqr.run(cb, sw.extent(), [&] {
          vee::cmd_push_vert_frag_constants(cb, *pl, &g_pc);
          vee::cmd_bind_descriptor_set(cb, *pl, 0, dset);
        });
      });
    });
  }
} t;
