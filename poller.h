/*
 * Public domain, 2013 Simone Basso.
 */

/* Methods that we only use internally: */

#ifdef __cplusplus
extern "C" {
#endif

struct event_base *NeubotPoller_event_base_(struct NeubotPoller *);
struct evdns_base *NeubotPoller_evdns_base_(struct NeubotPoller *);

#ifdef __cplusplus
}
#endif
