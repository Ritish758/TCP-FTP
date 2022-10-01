set terminal png
set output "Wired_TCP_Vegas.png"
set title "Throughput vs Packet size for TCP-Vegas"
set xlabel "Packet Size (in bytes)"
set ylabel "Throughput (in Kbps)"

set xrange [20:1520]
plot "-"  title "TCP-Vegas" with linespoints
40 205.891
44 208.055
48 208.965
52 211.512
60 212.977
552 656.278
576 668.548
628 736.664
1420 1985.6
1500 2122.71
e
