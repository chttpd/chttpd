# Lint
set(PRETTYC_FLAGS
    --recursive
    --verbose=0
    --repository=.
    --extensions=c,h,in
    --linelength=80
    --headers=h,in
    --includeorder=standardcfirst
    --root=${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}
    #--quiet
)
add_custom_target(lint 
  COMMAND prettyc ${PRETTYC_FLAGS}
)
