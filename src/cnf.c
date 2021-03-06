#include <stdlib.h>
#include <string.h>
#include "../include/cnf.h"

value* new_boolean(boolean b) {
  value* v = malloc(sizeof(value));
  v->nb_references = 1;
  v->type = constant;
  v->value.b = b;
  return v;
}

value* new_litteral(cnf* c) {
  value* v = malloc(sizeof(value));
  v->nb_references = 1;
  v->type = variable;
  v->value.l = ++(c->nb_litterals);
  return v;
}

value** copy_word(value** dest, value** src, int size) {
  int i;
  memcpy(dest, src, sizeof(value*) * size);
  for (i = 0; i < size; i++) {
    dest[i]->nb_references++;
  }
  return dest;
}

void free_value(value* v) {
  if (v->nb_references <= 1) {
    free(v);
  } else {
    v->nb_references--;
  }
}

void free_word(value** v, int size) {
  int i;
  for (i = 0; i < size; i++) {
    free_value(v[i]);
  }
  free(v);
}

cnf* new_cnf() {
  cnf* cnf = malloc(sizeof(struct cnf_t));
  cnf->nb_litterals = 0;
  cnf->head = NULL;
  return cnf;
}

cnf* copy_cnf(cnf* old_cnf) {
  clause * original, * copy;
  cnf* ret = malloc(sizeof(struct cnf_t));
  *ret = *old_cnf;
  if (old_cnf->head != NULL) {
    original = old_cnf->head;
    copy = malloc(sizeof(clause));
    *copy = *original;
    ret->head = copy;
    while (original->next != NULL) {
      original = original->next;
      copy->next = memcpy(malloc(sizeof(clause)), original, sizeof(clause));
      copy = copy->next;
    }
  }
  return ret;
}

void new_clause(cnf* cnf, litteral l_a, litteral l_b, litteral l_c, litteral l_d) {
  clause* cl = malloc(sizeof(clause));
  cl->litterals[0] = l_a;
  cl->litterals[1] = l_b;
  cl->litterals[2] = l_c;
  cl->litterals[3] = l_d;
  cl->max_litteral = cnf->nb_litterals;
  cl->next = cnf->head;
  cnf->head = cl;
}

cnf* fix_value(cnf* cnf, litteral l) {
  clause* cl = cnf->head;
  clause* pred = NULL;
  clause* toRelax = NULL;
  while (cl != NULL && cl->max_litteral >= abs(l)) {
    if (cl->litterals[0] == l || cl->litterals[1] == l || cl->litterals[2] == l || cl->litterals[3] == l) {
      /* We satisfy the clause */
      if (pred != NULL) { pred->next = cl->next; } else { cnf->head = cl->next; }
      toRelax = cl;
      cl = cl->next;
      free(toRelax);
    } else if (cl->litterals[0] == -l) {
      if (cl->litterals[1] == 0 && cl->litterals[2] == 0 && cl->litterals[3] == 0) {
        /* We unsatisfy the clause */
        if (pred != NULL) { pred->next = cl->next; } else { cnf->head = cl->next; }
        toRelax = cl;
        cl = cl->next;
        free(toRelax);
      } else {
        cl->litterals[0] = 0;
        pred = cl;
        cl = cl->next;
      }
    } else if (cl->litterals[1] == -l) {
      if (cl->litterals[0] == 0 && cl->litterals[2] == 0 && cl->litterals[3] == 0) {
        /* We unsatisfy the clause */
        if (pred != NULL) { pred->next = cl->next; } else { cnf->head = cl->next; }
        toRelax = cl;
        cl = cl->next;
        free(toRelax);
      } else {
        cl->litterals[1] = 0;
        pred = cl;
        cl = cl->next;
      }
    } else if (cl->litterals[2] == -l) {
      if (cl->litterals[0] == 0 && cl->litterals[1] == 0 && cl->litterals[3] == 0) {
        /* We unsatisfy the clause */
        if (pred != NULL) { pred->next = cl->next; } else { cnf->head = cl->next; }
        toRelax = cl;
        cl = cl->next;
        free(toRelax);
      } else {
        cl->litterals[2] = 0;
        pred = cl;
        cl = cl->next;
      }
    } else if (cl->litterals[3] == -l) {
      if (cl->litterals[0] == 0 && cl->litterals[1] == 0 && cl->litterals[2] == 0) {
        /* We unsatisfy the clause */
        if (pred != NULL) { pred->next = cl->next; } else { cnf->head = cl->next; }
        toRelax = cl;
        cl = cl->next;
        free(toRelax);
      } else {
        cl->litterals[3] = 0;
        pred = cl;
        cl = cl->next;
      }
    } else {
      pred = cl;
      cl = cl->next;
    }
  }
  return cnf;
}

void del_cnf(cnf* cnf) {
  clause* current;
  clause* next = cnf->head;
  while (next != NULL) {
    current = next;
    next = next->next;
    free(current);
  }
  free(cnf);
}

value* not(cnf* cnf, value* a) {
  value* r;
  if (a->type == constant) {
    if (a->value.b == false) {
      return new_boolean(true);
    } else {
      return new_boolean(false);
    }
  } else {
    /* ¬a -> (¬r V ¬a) & (r V a) */
    /* r = new_litteral(cnf);
    new_clause(cnf, r->value.l, a->value.l, 0);
    new_clause(cnf, -r->value.l, -a->value.l, 0); */
    r = memcpy(malloc(sizeof(value)), a, sizeof(value));
    r->nb_references = 1;
    r->value.l = -r->value.l;
    return r;
  }
}

value* or(cnf* cnf, value* a, value* b) {
  value* r;
  if (a->type == constant && b->type == constant) {
    return new_boolean(a->value.b || b->value.b);
  } else if (a->type == constant) {
    if (a->value.b == true) {
      a->nb_references++;
      return a;
    } else {
      b->nb_references++;
      return b;
    }
  } else if (b->type == constant) {
    if (b->value.b == true) {
      b->nb_references++;
      return b;
    } else {
      a->nb_references++;
      return a;
    }
  } else {
    /* a V b -> (¬r V a V b) & (r V ¬a) & (r V ¬b) */
    r = new_litteral(cnf);
    new_clause(cnf, -r->value.l, a->value.l, b->value.l, 0);
    new_clause(cnf, r->value.l, -a->value.l, 0, 0);
    new_clause(cnf, r->value.l, -b->value.l, 0, 0);
    return r;
  }
}

value* and(cnf* cnf, value* a, value* b) {
  value* r;
  if (a->type == constant && b->type == constant) {
    return new_boolean(a->value.b && b->value.b);
  } else if (a->type == constant) {
    if (a->value.b == true) {
      b->nb_references++;
      return b;
    } else {
      a->nb_references++;
      return a;
    }
  } else if (b->type == constant) {
    if (b->value.b == true) {
      a->nb_references++;
      return a;
    } else {
      b->nb_references++;
      return b;
    }
  } else {
    /* a & b -> (r V ¬a V ¬b) & (¬r V a) & (¬r V b) */
    r = new_litteral(cnf);
    new_clause(cnf, r->value.l, -a->value.l, -b->value.l, 0);
    new_clause(cnf, -r->value.l, a->value.l, 0, 0);
    new_clause(cnf, -r->value.l, b->value.l, 0, 0);
    return r;
  }
}

value* xor(cnf* cnf, value* a, value* b) {
  value* r;
  if (a->type == constant && b->type == constant) {
    return new_boolean(a->value.b ^ b->value.b);
  } else if (a->type == constant) {
    if (a->value.b == true) {
      return not(cnf, b);
    } else {
      b->nb_references++;
      return b;
    }
  } else if (b->type == constant) {
    if (b->value.b == true) {
      return not(cnf, a);
    } else {
      a->nb_references++;
      return a;
    }
  } else {
    /* a ^ b -> (a V b V ¬r) & (a V ¬b V r) & (¬a V b V r) & (¬a V ¬b V ¬r) */
    r = new_litteral(cnf);
    new_clause(cnf, -r->value.l, a->value.l, b->value.l, 0);
    new_clause(cnf, -r->value.l, -a->value.l, -b->value.l, 0);
    new_clause(cnf, r->value.l, -a->value.l, b->value.l, 0);
    new_clause(cnf, r->value.l, a->value.l, -b->value.l, 0);
    return r;
  }
}

value* xor_3(cnf* cnf, value* a, value* b, value* c) {
  value * r, * s1, * s2, * s3;
  if (a->type == constant && b->type == constant && c->type == constant) {
    return new_boolean(a->value.b ^ b->value.b ^ c->value.b);
  } else if (a->type == constant) {
    if (a->value.b == true) {
      return not(cnf, xor(cnf, b, c));
    } else {
      return xor(cnf, b, c);
    }
  } else if (b->type == constant) {
    if (b->value.b == true) {
      return not(cnf, xor(cnf, a, c));
    } else {
      return xor(cnf, a, c);
    }
  } else if (c->type == constant) {
    if (c->value.b == true) {
      return not(cnf, xor(cnf, a, b));
    } else {
      return xor(cnf, a, b);
    }
  } else {
    /* a + b + c -> (s1 V a V ¬b) & (s1 V ¬a V b) & (¬s1 V a V b) & (¬s1 V ¬a V ¬b)
                  & (s2 V a V ¬c) & (s2 V ¬a V c) & (¬s2 V a V c) & (¬s2 V ¬a V ¬c)
                  & (s3 V b V ¬c) & (s3 V ¬b V c) & (¬s3 V b V c) & (¬s3 V ¬b V ¬c)
                  & (r V s1 V ¬c) & (r V ¬s1 V c) & (¬r V s1 V c) & (¬r V ¬s1 V ¬c)
                  & (r V s2 V ¬b) & (r V ¬s2 V b) & (¬r V s2 V b) & (¬r V ¬s2 V ¬b)
                  & (r V s3 V ¬a) & (r V ¬s3 V a) & (¬r V s3 V a) & (¬r V ¬s3 V ¬a) */
    r = new_litteral(cnf);
    new_clause(cnf, r->value.l, -a->value.l, b->value.l, c->value.l);
    new_clause(cnf, r->value.l, a->value.l, -b->value.l, c->value.l);
    new_clause(cnf, r->value.l, a->value.l, b->value.l, -c->value.l);
    new_clause(cnf, r->value.l, -a->value.l, -b->value.l, -c->value.l);
    new_clause(cnf, -r->value.l, a->value.l, b->value.l, c->value.l);
    new_clause(cnf, -r->value.l, -a->value.l, -b->value.l, c->value.l);
    new_clause(cnf, -r->value.l, a->value.l, -b->value.l, -c->value.l);
    new_clause(cnf, -r->value.l, -a->value.l, b->value.l, -c->value.l);
    return r;
  }
}

value* ch(cnf* cnf, value* a, value* b, value* c) {
  value* r;
  if (a->type == constant && b->type == constant && c->type == constant) {
    return a->value.b ? new_boolean(b->value.b) : new_boolean(c->value.b);
  } else if (a->type == constant) {
    if (a->value.b == true) {
      b->nb_references++;
      return b;
    } else {
      c->nb_references++;
      return c;
    }
  } else if (b->type == constant) {
    if (b->value.b == true) {
      return or(cnf, a, c);
    } else {
      return and(cnf, not(cnf, a), c);
    }
  } else if (c->type == constant) {
    if (c->value.b == true) {
      return or(cnf, not(cnf, a), b);
    } else {
      return and(cnf, a, b);
    }
  } else {
    /* (a & b) V (¬a & c) -> (r V ¬a V ¬b) & (r V a V ¬c)
                           & (¬r V ¬a V b) & (¬r V a V c) */
    r = new_litteral(cnf);
    new_clause(cnf, r->value.l, -a->value.l, -b->value.l, 0);
    new_clause(cnf, r->value.l, a->value.l, -c->value.l, 0);
    new_clause(cnf, -r->value.l, -a->value.l, b->value.l, 0);
    new_clause(cnf, -r->value.l, a->value.l, c->value.l, 0);
    return r;
  }
}


value* maj(cnf* cnf, value* a, value* b, value* c) {
  value* r;
  if (a->type == constant && b->type == constant && c->type == constant) {
    return new_boolean(
      (a->value.b && b->value.b)
      ^ (b->value.b && c->value.b)
      ^ (a->value.b && c->value.b)
    );
  } else if (a->type == constant) {
    if (a->value.b == true) {
      return or(cnf, b, c);
    } else {
      return and(cnf, b, c);
    }
  } else if (b->type == constant) {
    if (b->value.b == true) {
      return or(cnf, a, c);
    } else {
      return and(cnf, a, c);
    }
  } else if (c->type == constant) {
    if (c->value.b == true) {
      return or(cnf, a, b);
    } else {
      return and(cnf, a, b);
    }
  } else {
    /* (a & b) V (b & c) V (a & c) ->
                (r V ¬a V ¬b) & (r V ¬b V ¬c) & (r V ¬a V ¬c)
              & (¬r V a V b) & (¬r V b V c) & (¬r V a V c) */
    r = new_litteral(cnf);
    new_clause(cnf, r->value.l, -a->value.l, -b->value.l, 0);
    new_clause(cnf, r->value.l, -b->value.l, -c->value.l, 0);
    new_clause(cnf, r->value.l, -a->value.l, -c->value.l, 0);
    new_clause(cnf, -r->value.l, a->value.l, b->value.l, 0);
    new_clause(cnf, -r->value.l, b->value.l, c->value.l, 0);
    new_clause(cnf, -r->value.l, a->value.l, c->value.l, 0);
    return r;
  }
}
