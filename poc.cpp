#pragma leco app
#pragma leco add_shader "poc.vert"
#pragma leco add_shader "poc.frag"

import casein;
import dotz;
import mtx;
import rng;
import sith;
import vee;
import voo;

static struct upc {
  dotz::vec2 aspect;
  dotz::vec2 displ {};
  float scale = 6;
} g_pc;

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

      ots_loop(dq, sw, [&](auto cb) {
        oqr.run(cb, sw.extent(), [&] {
          vee::cmd_push_vert_frag_constants(cb, *pl, &g_pc);
        });
      });
    });
  }
} t;
