#include <stdlib.h>
#include "../include/operations.h"

/*
 * Process a 32bits addition
 */
value** bool_add_32(cnf* cnf, value** a, value** b) {
  int i;
  value* r;
  value** res = malloc(sizeof(value*) * 32);
  r = and(cnf, a[31], b[31]);
  res[31] = xor(cnf, a[31], b[31]);
  /* res[31] = new_constant(get_val(a[31]) ^ get_val(b[31])); */
  for (i = 30; i >= 0; i--) {
    res[i] = xor(cnf, r, xor(cnf, a[i], b[i]));
    /* res[i] = new_constant(get_val(a[i]) ^ get_val(b[i]) ^ get_val(r)); */
    if (i != 0) {
      r = or(cnf,
        and(cnf, a[i], b[i]),
        or(cnf, and(cnf, a[i], r), and(cnf, b[i], r))
      );
    }
  }
  return res;
}

/*
 * Process a S0 word function
 */
value** bool_s_0(cnf* cnf, value** x) {
  int i = 0;
  value** res = malloc(sizeof(value*) * 32);
  for (; i < 3; i++) {
    /* a ^ b -> (a V b V ¬r) & (a V ¬b V r) & (¬a V b V r) & (¬a V ¬b V ¬r) */
    res[i] = xor(cnf, x[(i-7)&31], x[(i-18)&31]);
  }
  for (; i < 32; i++) {
    res[i] = xor(cnf,
      xor(cnf, x[(i-7)&31], x[(i-18)&31]),
      x[i-3]
    );
  }
  return res;
}

/*
 * Process a S1 word function
 */
value** bool_s_1(cnf* cnf, value** x) {
  int i = 0;
  value** res = malloc(sizeof(value*) * 32);
  for (; i < 10; i++) {
    res[i] = xor(cnf, x[(i-17)&31], x[(i-19)&31]);
  }
  for (; i < 32; i++) {
    res[i] = xor(cnf,
      xor(cnf, x[(i-17)&31], x[(i-19)&31]),
      x[i-10]
    );
  }
  return res;
}

/*
 * Process a E0 word function
 */
value** bool_e_0(cnf* cnf, value** x) {
  int i = 0;
  value** res = malloc(sizeof(value*) * 32);
  for (; i < 32; i++) {
    res[i] = xor(cnf,
      xor(cnf, x[(i-2)&31], x[(i-13)&31]),
      x[(i-22)&31]
    );
  }
  return res;
}

/*
 * Process a E1 word function
 */
value** bool_e_1(cnf* cnf, value** x) {
  int i = 0;
  value** res = malloc(sizeof(value*) * 32);
  for (; i < 32; i++) {
    res[i] = xor(cnf,
      xor(cnf, x[(i-6)&31], x[(i-11)&31]),
      x[(i-25)&31]
    );
  }
  return res;
}

/*
 * Process a Ch word function
 */
value** bool_ch(cnf* cnf, value** x, value** y, value** z) {
  int i = 0;
  value** res = malloc(sizeof(value*) * 32);
  for (; i < 32; i++) {
    res[i] = or(cnf, and(cnf, x[i], y[i]), and(cnf, not(cnf, x[i]), z[i]));
  }
  return res;
}

/*
 * Process a Maj word function
 */
value** bool_maj(cnf* cnf, value** x, value** y, value** z) {
  int i = 0;
  value** res = malloc(sizeof(value*) * 32);
  for (; i < 32; i++) {
    res[i] = or(cnf,
      or(cnf, and(cnf, x[i], y[i]), and(cnf, x[i], z[i])),
      and(cnf, y[i], z[i])
    );
  }
  return res;
}
