#pragma leco app
#pragma leco add_shader "poc.vert"
#pragma leco add_shader "poc.frag"

import casein;
import dotz;
import hai;
import rng;
import silog;
import vee;
import voo;

static constexpr const dotz::ivec2 nil { 10000 };

static struct upc {
  dotz::vec2 aspect;
  dotz::vec2 displ {};
  float scale = 6;
} g_pc;
static dotz::ivec2 g_selection = nil;

static constexpr const unsigned grid_size = 16;

static void load_grid(voo::h2l_buffer * buf) {
  voo::mapmem m { buf->host_memory() };
  auto ptr = static_cast<unsigned *>(*m);
  for (auto i = 0; i < grid_size * grid_size; i++, ptr++) *ptr = 0;
}

static void translate() {
  auto d = casein::mouse_rel / casein::window_size;
  g_pc.displ = g_pc.displ - d * 100.0;
}

struct init : public voo::casein_thread {
  init() : casein_thread {} {
    using namespace casein;
    handle(MOUSE_MOVE, M_WHEEL, translate);
    handle(MOUSE_DOWN, [] {
      silog::log(silog::debug, "%d %d", g_selection.x, g_selection.y);
    });
  }

  void run() {
    constexpr const auto fmt = vee::image_format_rgba_unorm;

    voo::device_and_queue dq { "poc" };
    auto pd = dq.physical_device();
    auto s = dq.surface();

    while (!interrupted()) {
      auto rp = vee::create_render_pass({{
        vee::create_colour_attachment(pd, s),
        vee::create_colour_attachment(fmt),
      }});
      voo::offscreen::colour_buffer cbuf { pd, voo::extent_of(pd, s), fmt };
      voo::offscreen::host_buffer hbuf { pd, { 1, 1 } };

      voo::swapchain_and_stuff sw { dq, *rp, {{ cbuf.image_view() }} };

      auto dsl = vee::create_descriptor_set_layout({
        vee::dsl_fragment_storage(),
      });
      vee::pipeline_layout pl = vee::create_pipeline_layout({ *dsl }, {
        vee::vert_frag_push_constant_range<upc>()
      });

      auto ks = hai::view { vee::specialisation_map_entry<unsigned>() };
      auto frag_k = vee::specialisation_info(&grid_size, ks);
      voo::one_quad oq { pd };
      auto p = vee::create_graphics_pipeline({
          .pipeline_layout = *pl,
          .render_pass = *rp,
          .blends {
            vee::colour_blend_classic(),
            vee::colour_blend_none(),
          },
          .shaders {
            voo::shader("poc.vert.spv").pipeline_vert_stage(),
            voo::shader("poc.frag.spv").pipeline_frag_stage("main", &frag_k),
          },
          .bindings { oq.vertex_input_bind() },
          .attributes { oq.vertex_attribute(0) },
      });

      constexpr const unsigned sz = grid_size * grid_size * sizeof(unsigned);
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
      auto max = dotz::ivec2 { grid_size } - min - g_pc.scale;
      g_pc.displ = (max + min) / 2.0;

      extent_loop(dq.queue(), sw, [&] {
        auto nxt = g_pc.displ;
        if (nxt.x > max.x) nxt.x -= (nxt.x - max.x) * 0.3;
        if (nxt.y > max.y) nxt.y -= (nxt.y - max.y) * 0.3;
        if (nxt.x < min.x) nxt.x -= (nxt.x - min.x) * 0.3;
        if (nxt.y < min.y) nxt.y -= (nxt.y - min.y) * 0.3;
        g_pc.displ = nxt;

        sw.queue_one_time_submit(dq.queue(), [&](auto pcb) {
          voo::cmd_render_pass scb {vee::render_pass_begin {
            .command_buffer = *pcb,
            .render_pass = *rp,
            .framebuffer = sw.framebuffer(),
            .extent = sw.extent(),
            .clear_colours {
              vee::clear_colour(0, 0, 0, 0),
              vee::clear_colour(0, 0, 0, 0),
            },
          }};
          vee::cmd_push_vert_frag_constants(*scb, *pl, &g_pc);
          vee::cmd_bind_descriptor_set(*scb, *pl, 0, dset);
          vee::cmd_set_viewport(*scb, sw.extent());
          vee::cmd_set_scissor(*scb, sw.extent());
          vee::cmd_bind_gr_pipeline(*scb, *p);
          oq.run(*scb, 0, 1);

          int mx = casein::mouse_pos.x * casein::screen_scale_factor;
          int my = casein::mouse_pos.y * casein::screen_scale_factor;
          cbuf.cmd_copy_to_host(*scb, { mx, my }, { 1, 1 }, hbuf.buffer());
        });

        auto mem = hbuf.map();
        auto pick = static_cast<unsigned char *>(*mem);
        if (pick[3]) g_selection = { pick[0], pick[1] };
        else g_selection = nil;
      });
    }
  }
} t;
