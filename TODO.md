## high priority
- http 1.1 vs 2.0

- request parsing
  * query string
  * json form parsing
- default handler


## backlog
- user agent
- http accept header
- http decoding/encoding
- gcov
- profile using valgrind
- make cpack
- make test
- buffsize should be multiple of pagesize. if not, raise error
- separate buffer size options for request and response
- handle multiple requests with one connection using coro_reset and keep-alive
  header
- gzip, deflate
- url formencodded 
- multipart
- cookie
- etag
- access log: one line per request
- unix domain socket
- hooks: init, deinit, new request, new connection and etc.
