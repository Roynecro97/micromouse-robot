#ifndef UNITTESTS_HACK_H
#define UNITTESTS_HACK_H

#define REGISTER_TEST_FILE(ident) int ident = 0

#define LOAD_TEST_FILE(ident)                                                                                          \
    extern int ident;                                                                                                  \
    [[maybe_unused]] static int use_##ident = ident

#endif  // UNITTESTS_HACK_H
