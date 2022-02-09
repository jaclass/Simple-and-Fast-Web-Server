# $1: the port of the server
# $2: the output path for the results

port=$1
output=$2
req_count="1000"
cgi_path="/cgi-bin/adder?11\&29"
grep_metrics="Concurrency Level|Failed requests|Requests per second|Time per request|Transfer rate"
concurrencies=(1 5 10 20 40 80 100 120)

evalFunc(){
    echo "==================================== Concurrency $c ====================================" >> $output
    ab -c "$1" -n "$req_count" localhost:"$port""$cgi_path" | grep -E "$grep_metrics" >> $output
    echo "=======================================================================================" >> $output
    echo "" >> $output
}

for c in ${concurrencies[@]}
do
    evalFunc $c
done





