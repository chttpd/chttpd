#ifndef INCLUDE_CHTTPD_CHTTPD_H_
#define INCLUDE_CHTTPD_CHTTPD_H_


typedef struct chttpd *chttpd_t;
struct chttpd_config {
    const char *bind;
    unsigned short backlog;
};


void
chttpd_config_default(struct chttpd_config *c);


struct chttpd *
chttpd_new(struct chttpd_config *c);


int
chttpd_main(struct chttpd *s);


int
chttpdA(int argc, void *argv[]);


#endif  // INCLUDE_CHTTPD_CHTTPD_H_
