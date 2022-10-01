set terminal png
set output "Wired_TCP_Westwood.png"
set title "Throughput vs Packet size for TCP-Westwood"
set xlabel "Packet Size (in bytes)"
set ylabel "Throughput (in Kbps)"

set xrange [20:1520]
plot "-"  title "TCP-Westwood" with linespoints
40 1003.2
44 959.859
48 923.768
52 893.195
60 884.629
552 2754.96
576 3020.39
628 3325.74
1420 5023.22
1500 5042.34
e
