echo -e "Start of testing speed: \n"

start_NO_threads=1

DEFAULT_MAX_NO_THREADS=16
max_NO_threads=${1:-${DEFAULT_MAX_NO_THREADS}}

# cycle of running calculation in threads (from 1 to 12) 
# my pc has 8 cores so the time result will decrease until the number of threads reaches 8
# after time should not increase 


for NO_threads in $(eval echo "{$start_NO_threads..$max_NO_threads}")
do
    echo "NO threads: $NO_threads"
    # flag -v for full info

        # this call doesn't portable !
        # /usr/bin/time -f "\n\tcalculation time: %e sec\n" ./xxx $NO_threads
    time ./xxx $NO_threads
    echo -e "\n"
done

echo -e "END OF TESTS\n\n"
