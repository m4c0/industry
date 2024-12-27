#pragma leco app
#pragma leco add_shader "poc.vert"
#pragma leco add_shader "poc.frag"

import casein;
import mtx;
import rng;
import sith;
import vee;
import voo;

struct : public voo::casein_thread {
  void run() {
    main_loop("poc-voo", [&](auto & dq, auto & sw) {
      vee::pipeline_layout pl = vee::create_pipeline_layout();
      voo::one_quad_render oqr { "poc", &dq, *pl };

      ots_loop(dq, sw, [&](auto cb) {
        oqr.run(cb, sw.extent());
      });
    });
  }
} t;
