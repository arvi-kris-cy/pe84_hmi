/* Sensory Confidential
 * Copyright (C)2017-2024 Sensory, Inc. https://sensory.com/
 *
 * Custom initialization for TrulyNatural SDK 7.3.x
 * created from music_player-ethos-u55-128-infineon-ram.snsr
 *
 * Include this file in the build, and compile with:
 *
 *    -DSNSR_USE_SUBSET
 *
 * See the TrulyNatural SDK documentation for additional detail.
 *------------------------------------------------------------------------------
 */

#if defined(__cplusplus)
extern "C" {
#endif

#include <snsr.h>

#if SNSR_VERSION_MAJOR != 7 || SNSR_VERSION_MINOR != 3
# error Incorrect TrulyNatural SDK version, initialization code requires 7.3.x
#endif

SnsrRC dataFlow_register_acmodelTflite(void);
SnsrRC d510a(void);
SnsrRC x512d(void);
SnsrRC r513p(void);
SnsrRC q515e(void);
SnsrRC r516r(void);
SnsrRC m518l(void);
SnsrRC s519n(void);
SnsrRC r51bv(void);
SnsrRC x520r(void);
SnsrRC e523a(void);
SnsrRC n526d(void);
SnsrRC t52aa(void);
SnsrRC i52er(void);
SnsrRC c52ch(void);
SnsrRC m52fz(void);
SnsrRC t530k(void);
SnsrRC v534l(void);
SnsrRC o535g(void);
SnsrRC v538g(void);
SnsrRC k53fo(void);
SnsrRC g540x(void);
SnsrRC x542n(void);
SnsrRC n544v(void);
SnsrRC u545z(void);
SnsrRC y547n(void);
SnsrRC i54av(void);
SnsrRC r7f1s(SnsrSession *s, const char *v, unsigned f);

SnsrRC snsrNewSubset(SnsrSession *session, const char *version)
{
  SnsrRC r = SNSR_RC_OK;
  SnsrRC (*init[])(void) = {
    dataFlow_register_acmodelTflite, d510a, x512d, r513p, q515e,
    r516r, m518l, s519n, r51bv, x520r, e523a, n526d, t52aa, i52er,
    c52ch, m52fz, t530k, v534l, o535g, v538g, k53fo, g540x, x542n,
    n544v, u545z, y547n, i54av,
    NULL
  };
  int i;
  for (i = 0; init[i]; i++)
    if ((r = init[i]()) != SNSR_RC_OK) break;
  if (r != SNSR_RC_OK) return SNSR_RC_ELEMENT_INIT_FAILED;
  return r7f1s(session, version, 1);
}

#if defined(__cplusplus)
}
#endif
