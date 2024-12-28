#pragma leco app
#pragma leco add_shader "poc.vert"
#pragma leco add_shader "poc.frag"

import casein;
import dotz;
import vee;
import voo;

static struct upc {
  dotz::vec2 aspect;
  dotz::vec2 displ {};
  float scale = 6;
} g_pc;

static constexpr const dotz::vec2 grid_size { 16 };

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
      vee::pipeline_layout pl = vee::create_pipeline_layout({
        vee::vert_frag_push_constant_range<upc>()
      });
      voo::one_quad_render oqr { "poc", &dq, *pl };

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
        });
      });
    });
  }
} t;
