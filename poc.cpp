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
  float scale = 6;
} g_pc;

struct : public voo::casein_thread {
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
