#!/bin/bash
echo "-> using log files named \"$1\""
export FILENAMEPREFIX="$1"
echo $FILENAMEPREFIX

gnuplot <<EOF

pricerangemax = `grep '^[0-9]' ${FILENAMEPREFIX}_traces.txt | sort -n -k5,5 -t, | tail -n 1 | awk -F "[ ]*,[ ]*" '{print \$5}'`


#==============================
reset

set terminal png size 800, 600
set datafile separator ","
set output "plot_feat_time.png"
set key off
set title "Distinct users involved in a transaction in the given time frame"

set ytics border nomirror out font 'Arial,9'
set xrange [0:86400]
set grid xtics
set grid ytics
set ylabel "number of users"
set xlabel "time (window size: 15 min)"
set xtics font 'Arial,9'
set xtics border nomirror out scale 1.0 ("00:00" 0, "02:00" 7200, "04:00" 14400, "06:00" 21600, "08:00" 28800, "10:00" 36000, "12:00" 43200,\
 "14:00" 50400, "16:00" 57600, "18:00" 64800, "20:00" 72000, "22:00" 79200, "24:00" 86400)
plot '${FILENAMEPREFIX}_featOverview.txt' index 1 using ((\$3 + \$2)/2.0):4:(\$3 - \$2) with boxes fs pattern 2 notitle lt 1


#==============================
reset

set terminal png size 800, 600
set datafile separator ","
set output "plot_feat_amount.png"
set key off
set title "Distinct users involved in a transaction for the given amount"

set ytics border nomirror out font 'Arial,9'
set grid xtics
set grid ytics
set ylabel "number of users"
set xlabel "amount (window size: 10 BTC)"
set xtics font 'Arial,9'
set xtics border nomirror out scale 1.0
plot '${FILENAMEPREFIX}_featOverview.txt' index 0 using ((\$3 + \$2)/2.0):4:(\$3 - \$2) with boxes fs pattern 2 notitle lt 1


#==============================
reset

set terminal png size 800, 800
set datafile separator ","
set output "plot_traces_profiles.png"
set key box
set key font 'Arial,9'
#set title "Transactions issued by users (no stores, etc.)"
set tmargin 0
set bmargin 0.8
set rmargin 1.6
set lmargin 9
set multiplot layout 3, 1 title "Transactions issued by three selected users per profile"

set xrange [0:86400]
#set yrange [0:(pricerangemax+20)]
set ytics border nomirror out font 'Arial,9'
unset xtics


set ytics border nomirror out font 'Arial,9'
#set ytics 0.5

set xtics border nomirror out scale 0.0 ("" 0, "" 7200, "" 14400, "" 21600, "" 28800, "" 36000, "" 43200,\
 "" 50400, "" 57600, "" 64800, "" 72000, "" 79200, "" 86400)
set grid xtics
set key inside left
set ylabel " "
plot "${FILENAMEPREFIX}_traces.txt_profile_0.txt" index 0 using 7:5 title "professor 1" w p lc 1,\
     "" index 3 using 7:5 title "professor 2" w p lc 2,\
     "" index 6 using 7:5 title "professor 3" w p lc 3
set ylabel "amount in BTC"
set ylabel offset +2.8,+5
plot "${FILENAMEPREFIX}_traces.txt_profile_1.txt" index 0 using 7:5 title "phd student 1" w p lc 1,\
     "" index 3 using 7:5 title "phd student 2" w p lc 2,\
     "" index 6 using 7:5 title "phd student 3" w p lc 3
set bmargin 2.9
set ylabel " "
set xlabel "daytime"
set xtics font 'Arial,9'
set xtics border nomirror out scale 1.0 ("00:00" 0, "02:00" 7200, "04:00" 14400, "06:00" 21600, "08:00" 28800, "10:00" 36000, "12:00" 43200,\
 "14:00" 50400, "16:00" 57600, "18:00" 64800, "20:00" 72000, "22:00" 79200, "24:00" 86400)
set mxtics 8
set xlabel offset 0,+0.4
plot "${FILENAMEPREFIX}_traces.txt_profile_2.txt" index 0 using 7:5 title "student 1" w p lc 1,\
     "" index 3 using 7:5 title "student 2" w p lc 2,\
     "" index 6 using 7:5 title "student 3" w p lc 3

unset multiplot
set key nobox
unset yrange

#=======================
reset

set terminal png size 1400, 800
set datafile separator ","
set output "plot_traces_profile0_professors_collaborators.png"

set key box
set tmargin 0
set bmargin 0.8
set rmargin 1.6
set lmargin 9

set multiplot layout 3, 1 title "Collaborators from selected professors"

set ytics font 'Arial,9'
set ytics border mirror in scale 0.5 10.0
unset xtics

set boxwidth 0.85
set grid xtics
set grid ytics
set xtics border nomirror out offset 0,0 scale 0.0 1.0
set xtics font 'Arial,0'
set key inside left
set ylabel " "
#set table "profile0_collaborators.txt"
set xrange [-1:29]
plot "${FILENAMEPREFIX}_traces.txt_profile_0.txt" index 2 using 1:4:5 with boxes fs pattern 1 lc variable title "professor 1"
set ylabel "number of transactions"
set ylabel offset +2.8,+5
plot "${FILENAMEPREFIX}_traces.txt_profile_0.txt" index 5 using 1:4:5 with boxes fs pattern 1 lc variable title "professor 2"
set bmargin 2.9
set ylabel " "
set xlabel "collaborators"
set xtics font 'Arial,9'
set xtics border nomirror out rotate by -22 scale 1.0 1.0
set xlabel offset 0,+0.4
plot "${FILENAMEPREFIX}_traces.txt_profile_0.txt" index 8 using 1:4:5:xticlabels(3) with boxes fs pattern 1 lc variable title "professor 3"
#unset table

unset multiplot
unset grid
unset ytics

#========================
reset

set terminal png size 1400, 800
set datafile separator ","
set output "plot_traces_profile0_professors.png"
set key box
set tmargin 0
set bmargin 0.8
set rmargin 1.6
set lmargin 9
set multiplot layout 3, 1 title "Transactions issued by three selected professors"

set xrange [0:86400]
#set yrange [0:(pricerangemax+20)]
set ytics border nomirror out font 'Arial,9'
#set label font 'Arial,9'
unset xtics


set ytics border nomirror out font 'Arial,9'
#set ytics 0.5
unset xlabel
set xtics border nomirror out scale 0.0 ("" 0, "" 7200, "" 14400, "" 21600, "" 28800, "" 36000, "" 43200,\
 "" 50400, "" 57600, "" 64800, "" 72000, "" 79200, "" 86400)
set grid xtics
set key inside left
set ylabel " "
plot "${FILENAMEPREFIX}_traces.txt_profile_0.txt" index 1 using ((int(\$6)==1)? \$1 : -1000):2:(1+\$5) with impulses lc variable notitle,\
     "" index 0 using 7:5:(1+\$8) title "professor 1" w p lc variable pt 2,\
     "" index 1 using 1:3:4 with labels right font 'Arial,9' notitle
set ylabel "amount in BTC"
set ylabel offset +2.8,+5
plot "${FILENAMEPREFIX}_traces.txt_profile_0.txt" index 4 using ((int(\$6)==1)? \$1 : -1000):2:(1+\$5) with impulses lc variable notitle,\
     "" index 3 using 7:5:(1+\$8) title "professor 2" w p lc variable pt 2,\
     "" index 4 using 1:3:4 with labels right font 'Arial,9' notitle
set bmargin 2.9
set ylabel " "
set xlabel "daytime"
set xtics font 'Arial,9'
set xtics border nomirror out scale 1.0 ("00:00" 0, "02:00" 7200, "04:00" 14400, "06:00" 21600, "08:00" 28800, "10:00" 36000, "12:00" 43200,\
 "14:00" 50400, "16:00" 57600, "18:00" 64800, "20:00" 72000, "22:00" 79200, "24:00" 86400)
set mxtics 8
set xlabel offset 0,+0.4
plot "${FILENAMEPREFIX}_traces.txt_profile_0.txt" index 7 using ((int(\$6)==1)? \$1 : -1000):2:(1+\$5) with impulses lc variable notitle,\
     "" index 6 using 7:5:(1+\$8) title "professor 3" w p lc variable pt 2,\
     "" index 7 using 1:3:4 with labels right font 'Arial,9' notitle

unset multiplot
set key nobox
unset yrange

#=======================
reset

set terminal png size 1400, 800
set datafile separator ","
set output "plot_traces_profile1_phdstudents.png"
set key box
set tmargin 0
set bmargin 0.8
set rmargin 1.6
set lmargin 9
set multiplot layout 3, 1 title "Transactions issued by three selected phd students"

set xrange [0:86400]
#set yrange [0:(pricerangemax+20)]
set ytics border nomirror out font 'Arial,9'
unset xtics


set ytics border nomirror out font 'Arial,9'
#set ytics 0.5
unset xlabel
set key inside left
set xtics border nomirror out scale 0.0 ("" 0, "" 7200, "" 14400, "" 21600, "" 28800, "" 36000, "" 43200,\
 "" 50400, "" 57600, "" 64800, "" 72000, "" 79200, "" 86400)
set grid xtics
set ylabel " "
plot "${FILENAMEPREFIX}_traces.txt_profile_1.txt" index 1 using ((int(\$6)==1)? \$1 : -1000):2:(1+\$5) with impulses lc variable notitle,\
     "" index 0 using 7:5:(1+\$8) title "phd student 1" w p lc variable pt 2,\
     "" index 1 using 1:3:4 with labels right font 'Arial,9' notitle
set ylabel "amount in BTC"
set ylabel offset +2.8,+5
plot "${FILENAMEPREFIX}_traces.txt_profile_1.txt" index 4 using ((int(\$6)==1)? \$1 : -1000):2:(1+\$5) with impulses lc variable notitle,\
     "" index 3 using 7:5:(1+\$8) title "phd student 2" w p lc variable pt 2,\
     "" index 4 using 1:3:4 with labels right font 'Arial,9' notitle
set bmargin 2.9
set ylabel " "
set xlabel "daytime"
set xtics font 'Arial,9'
set xtics border nomirror out scale 1.0 ("00:00" 0, "02:00" 7200, "04:00" 14400, "06:00" 21600, "08:00" 28800, "10:00" 36000, "12:00" 43200,\
 "14:00" 50400, "16:00" 57600, "18:00" 64800, "20:00" 72000, "22:00" 79200, "24:00" 86400)
set mxtics 8
set xlabel offset 0,+0.4
plot "${FILENAMEPREFIX}_traces.txt_profile_1.txt" index 7 using ((int(\$6)==1)? \$1 : -1000):2:(1+\$5) with impulses lc variable notitle,\
     "" index 6 using 7:5:(1+\$8) title "phd student 3" w p lc variable pt 2,\
     "" index 7 using 1:3:4 with labels right font 'Arial,9' notitle
unset multiplot
set key nobox
unset yrange

#=======================
reset

set terminal png size 1400, 800
set datafile separator ","
set output "plot_traces_profile2_students.png"
set key box
set tmargin 0
set bmargin 0.8
set rmargin 1.6
set lmargin 9
set multiplot layout 3, 1 title "Transactions issued by three selected students"

set xrange [0:86400]
#set yrange [0:(pricerangemax+20)]
set ytics border nomirror out font 'Arial,9'
unset xtics


set ytics border nomirror out font 'Arial,9'
#set ytics 0.5
unset xlabel
set key inside left
set xtics border nomirror out scale 0.0 ("" 0, "" 7200, "" 14400, "" 21600, "" 28800, "" 36000, "" 43200,\
 "" 50400, "" 57600, "" 64800, "" 72000, "" 79200, "" 86400)
set grid xtics
set ylabel " "
plot "${FILENAMEPREFIX}_traces.txt_profile_2.txt" index 1 using ((int(\$6)==1)? \$1 : -1000):2:(1+\$5) with impulses lc variable notitle,\
     "" index 0 using 7:5:(1+\$8) title "student 1" w p lc variable pt 2,\
     "" index 1 using 1:3:4 with labels right font 'Arial,9' notitle
set ylabel "amount in BTC"
set ylabel offset +2.8,+5
plot "${FILENAMEPREFIX}_traces.txt_profile_2.txt" index 4 using ((int(\$6)==1)? \$1 : -1000):2:(1+\$5) with impulses lc variable notitle,\
     "" index 3 using 7:5:(1+\$8) title "student 2" w p lc variable pt 2,\
     "" index 4 using 1:3:4 with labels right font 'Arial,9' notitle
set bmargin 2.9
set ylabel " "
set xlabel "daytime"
set xtics font 'Arial,9'
set xtics border nomirror out scale 1.0 ("00:00" 0, "02:00" 7200, "04:00" 14400, "06:00" 21600, "08:00" 28800, "10:00" 36000, "12:00" 43200,\
 "14:00" 50400, "16:00" 57600, "18:00" 64800, "20:00" 72000, "22:00" 79200, "24:00" 86400)
set mxtics 8
set xlabel offset 0,+0.4
plot "${FILENAMEPREFIX}_traces.txt_profile_2.txt" index 7 using ((int(\$6)==1)? \$1 : -1000):2:(1+\$5) with impulses lc variable notitle,\
     "" index 6 using 7:5:(1+\$8) title "student 3" w p lc variable pt 2,\
     "" index 7 using 1:3:4 with labels right font 'Arial,9' notitle
unset multiplot
set key nobox
unset yrange

#=======================
reset

set terminal png size 800, 800
set datafile separator ","
set output "plot_traces_overall.png"
set key box
#set title "Transactions issued by users (no stores, etc.)"
set tmargin 0
set bmargin 0.8
set rmargin 1.6
set lmargin 9
set multiplot layout 4, 1 title "Transactions issued by users (no stores, etc.)"
#set xdata time
#set timefmt "%H:%M:%S"
#set format x "%H:%M"

#set xrange [ 0 : 20 ]
#set yrange [ 0 : 2 ]
#set mxtics 5
#set mytics 5

set xrange [0:86400]
set yrange [0:(pricerangemax+20)]
set ytics border nomirror out font 'Arial,9'
unset xtics


set ytics border nomirror out font 'Arial,9'
#set ytics 0.5
unset xlabel
set key inside left
set xtics border nomirror out scale 0.0 ("" 0, "" 7200, "" 14400, "" 21600, "" 28800, "" 36000, "" 43200,\
 "" 50400, "" 57600, "" 64800, "" 72000, "" 79200, "" 86400)
set grid xtics
set ylabel " "
plot "${FILENAMEPREFIX}_traces.txt" using 7:5 title "all users" w p lc 1
plot "${FILENAMEPREFIX}_traces.txt" using ((int(\$3)==0)? \$7 : -1000):5 title "professors" w p lc 2
set ylabel "amount in BTC"
set ylabel offset +2.8,+5
plot "${FILENAMEPREFIX}_traces.txt" using ((int(\$3)==1)? \$7 : -1000):5 title "phd students" w p lc 3
set bmargin 2.9
set ylabel " "
set xlabel "daytime"
set xtics font 'Arial,9'
set xtics border nomirror out scale 1.0 ("00:00" 0, "02:00" 7200, "04:00" 14400, "06:00" 21600, "08:00" 28800, "10:00" 36000, "12:00" 43200,\
 "14:00" 50400, "16:00" 57600, "18:00" 64800, "20:00" 72000, "22:00" 79200, "24:00" 86400)
set mxtics 8
set xlabel offset 0,+0.4
plot "${FILENAMEPREFIX}_traces.txt" using ((int(\$3)==2)? \$7 : -1000):5 title "students" w p lc 4

unset multiplot
set key nobox
unset yrange

#=======================

numberofprofessors = `grep '^.*professor.*[0-9]\{1,\}\$' ${FILENAMEPREFIX}_statistics.txt | grep -oPe "[0-9]+"`
numberofphdstudents = `grep '^.*phdstudent.*[0-9]\{1,\}\$' ${FILENAMEPREFIX}_statistics.txt | grep -oPe "[0-9]+"`
numberofstudents = `grep '^.* student.*[0-9]\{1,\}\$' ${FILENAMEPREFIX}_statistics.txt | grep -oPe "[0-9]+"`
numberofusers = numberofprofessors+numberofphdstudents+numberofstudents

#=======================
reset

set terminal png size 800, 800
set datafile separator ","
set output "plot_traces_overall_time.png"
unset title
set tmargin 0
set bmargin 0.8
set rmargin 1.6
set lmargin 9
set multiplot layout 4, 1 #title "Number of Transactions issued by users (no stores, etc.)"

set xrange [0:86400]
set ytics border nomirror out font 'Arial,9'


#15min window size
binwidth=900
bin(x,width)=width*floor(x/width) + (width/2)
set boxwidth binwidth

unset ylabel
unset xlabel
unset xtics

set table 'overall_time_plot.txt'
plot "${FILENAMEPREFIX}_traces.txt" using (bin(\$7,binwidth)):(1.0) smooth freq with boxes title "all users" lt 1
plot "${FILENAMEPREFIX}_traces.txt" using ((int(\$3)==0)? (bin(\$7,binwidth)) : -1000.0):(1.0) smooth freq with boxes title "professors" lt 2
plot "${FILENAMEPREFIX}_traces.txt" using ((int(\$3)==1)? (bin(\$7,binwidth)) : -1000.0):(1.0) smooth freq with boxes title "phd students" lt 3
plot "${FILENAMEPREFIX}_traces.txt" using ((int(\$3)==2)? (bin(\$7,binwidth)) : -1000.0):(1.0) smooth freq with boxes title "students" lt 4
unset table

set datafile separator whitespace

set key inside right
set xtics border nomirror out scale 0.0 ("" 0, "" 7200, "" 14400, "" 21600, "" 28800, "" 36000, "" 43200,\
 "" 50400, "" 57600, "" 64800, "" 72000, "" 79200, "" 86400)
set ylabel " "
set tmargin 1
plot "overall_time_plot.txt" index 0 using 1:(\$2/numberofusers) with boxes title "all users" lt 1
set tmargin 0
plot "overall_time_plot.txt" index 1 using 1:(\$2/numberofprofessors) with boxes title "professors" lt 2
set ylabel "avg. number of transactions"
set ylabel offset +1.5,+4
plot "overall_time_plot.txt" index 2 using 1:(\$2/numberofphdstudents) with boxes title "phd students" lt 3
set bmargin 2.9
set xtics border nomirror out scale 1.0 ("00:00" 0, "02:00" 7200, "04:00" 14400, "06:00" 21600, "08:00" 28800, "10:00" 36000, "12:00" 43200,\
 "14:00" 50400, "16:00" 57600, "18:00" 64800, "20:00" 72000, "22:00" 79200, "24:00" 86400)
set xtics font 'Arial,9'
set mxtics 8
set ylabel " "
set xlabel "daytime (window size: 15min)"
set xlabel offset 0,+0.4
plot "overall_time_plot.txt" index 3 using 1:(\$2/numberofstudents) with boxes title "students" lt 4

unset multiplot
set datafile separator ","

#=======================
reset

set terminal png size 800, 800
set datafile separator ","
set output "plot_traces_overall_amount.png"
unset title
set tmargin 0
set bmargin 0.8
set rmargin 1.6
set lmargin 9
unset multiplot


unset yrange
#set yrange [ 1 : 5]

set xrange [0:(pricerangemax+20)]
set ytics border nomirror out font 'Arial,9'


#15min window size
binwidth=10
bin(x,width)=width*floor(x/width) + (width/2)
set boxwidth binwidth

unset ylabel
unset xlabel
unset xtics

set table 'overall_amount_plot.txt'

set ylabel " "
plot "${FILENAMEPREFIX}_traces.txt" using (bin(\$5,binwidth)):(1.0) smooth freq with boxes title "all users" lt 1
plot "${FILENAMEPREFIX}_traces.txt" using ((int(\$3)==0)? (bin(\$5,binwidth)) : -1000.0):(1.0) smooth freq with boxes title "professors" lt 2
plot "${FILENAMEPREFIX}_traces.txt" using ((int(\$3)==1)? (bin(\$5,binwidth)) : -1000.0):(1.0) smooth freq with boxes title "phd students" lt 3
plot "${FILENAMEPREFIX}_traces.txt" using ((int(\$3)==2)? (bin(\$5,binwidth)) : -1000.0):(1.0) smooth freq with boxes title "students" lt 4

unset table

set datafile separator whitespace

set multiplot layout 4, 1 #title "Number of Transactions issued by users (no stores, etc.)"
set logscale y
set tmargin 1
set xtics border nomirror out scale 0.0 binwidth*5
set xtics font 'Arial,0'
plot 'overall_amount_plot.txt' index 0 using 1:(\$2/numberofusers) with boxes title "all users" lt 1
set tmargin 0
plot 'overall_amount_plot.txt' index 1 using 1:(\$2/numberofprofessors) with boxes title "professors" lt 2
set ylabel "avg. number of transactions (log scale)"
set ylabel offset +2.8,+5
plot 'overall_amount_plot.txt' index 2 using 1:(\$2/numberofphdstudents) with boxes title "phd students" lt 3
set bmargin 2.9
set xtics border nomirror out scale 1.0 binwidth*5
set xtics font 'Arial,9'
set mxtics 5
set ylabel " "
set xlabel "amount (window size: 10 BTC)"
set xlabel offset 0,+0.4
plot 'overall_amount_plot.txt' index 3 using 1:(\$2/numberofstudents) with boxes title "students" lt 4
unset multiplot

EOF


echo "-> done plotting."

