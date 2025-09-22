#ifndef INCLUDE_CHTTPD_CHTTPD_H_
#define INCLUDE_CHTTPD_CHTTPD_H_


struct chttpd_config {
    const char *bind;
    unsigned short backlog;
};


void
chttpd_config_default(struct chttpd_config *c);


int
chttpd_main(struct chttpd_config *c);


#endif  // INCLUDE_CHTTPD_CHTTPD_H_
