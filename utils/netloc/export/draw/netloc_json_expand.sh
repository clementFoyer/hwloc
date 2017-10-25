#!/bin/bash

function usage() {
    echo "Usage: $1 <path_to_json_file>"
}

if [ $# -lt 1 ]
then
    usage $(basename $0)
    exit -1
fi

for jsonfile in $@
do
    sed  -e 's/]}$/]\n}\n/'                                \
         -e 's/"hwloctopos":/\n  &/'                       \
         -e 's/]}],"partitions"/]}\n  ],\n  "partitions"/' \
         -e 's/]}],"paths":/]}\n  ],\n  "paths":/'         \
         -e 's/]}],"links"/]}\n  ],\n  "links"/'           \
         -e 's/}],"edges"/}\n  ],\n  "edges"/'             \
         -e 's/"nodes"/\n  &/'                             \
         -e 's/{"type":/{\n  "type":/'                     \
         -e 's/},{"id":/},\n    {"id":/g'                  \
         -e 's/\[{"id"/[\n    {"id"/g'                     \
         -i $jsonfile
done
