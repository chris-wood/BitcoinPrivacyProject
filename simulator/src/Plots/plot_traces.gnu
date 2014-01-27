pricerangemax = `grep '^[0-9]' "../"$FILENAMEPREFIX"_traces.txt" | sort -n -k4,4 -t, | tail -n 1 | awk -F "[ ]*,[ ]*" '{print $4}'`
#==============================
set terminal png size 800, 800
set datafile separator ","
set output "plot_traces_profiles.png"
set key box
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
plot "../$FILENAMEPREFIX_traces.txt_profile_0.txt" index 0 using 6:4 title "professor 1" w p lc 1,\
     "../$FILENAMEPREFIX_traces.txt_profile_0.txt" index 1 using 6:4 title "professor 2" w p lc 2,\
     "../$FILENAMEPREFIX_traces.txt_profile_0.txt" index 2 using 6:4 title "professor 3" w p lc 3
set ylabel "amount in BTC"
set ylabel offset +2.8,+5
plot "../$FILENAMEPREFIX_traces.txt_profile_1.txt" index 0 using 6:4 title "phd student 1" w p lc 1,\
     "../$FILENAMEPREFIX_traces.txt_profile_1.txt" index 1 using 6:4 title "phd student 2" w p lc 2,\
     "../$FILENAMEPREFIX_traces.txt_profile_1.txt" index 2 using 6:4 title "phd student 3" w p lc 3
set bmargin 2.9
set ylabel " "
set xlabel "daytime"
set xtics font 'Arial,9'
set xtics border nomirror out scale 1.0 ("00:00" 0, "02:00" 7200, "04:00" 14400, "06:00" 21600, "08:00" 28800, "10:00" 36000, "12:00" 43200,\
 "14:00" 50400, "16:00" 57600, "18:00" 64800, "20:00" 72000, "22:00" 79200, "24:00" 86400)
set mxtics 8
set xlabel offset 0,+0.4
plot "../$FILENAMEPREFIX_traces.txt_profile_2.txt" index 0 using 6:4 title "student 1" w p lc 1,\
     "../$FILENAMEPREFIX_traces.txt_profile_2.txt" index 1 using 6:4 title "student 2" w p lc 2,\
     "../$FILENAMEPREFIX_traces.txt_profile_2.txt" index 2 using 6:4 title "student 3" w p lc 3

unset multiplot
set key nobox
unset yrange

#=======================

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
unset xtics


set ytics border nomirror out font 'Arial,9'
#set ytics 0.5
unset xlabel
set xtics border nomirror out scale 0.0 ("" 0, "" 7200, "" 14400, "" 21600, "" 28800, "" 36000, "" 43200,\
 "" 50400, "" 57600, "" 64800, "" 72000, "" 79200, "" 86400)
set grid xtics
set key inside left
set ylabel " "
plot "../$FILENAMEPREFIX_traces.txt_profile_0.txt" index 0 using 6:4 title "professor 1" w p lc 1
set ylabel "amount in BTC"
set ylabel offset +2.8,+5
plot "../$FILENAMEPREFIX_traces.txt_profile_0.txt" index 1 using 6:4 title "professor 2" w p lc 1
set bmargin 2.9
set ylabel " "
set xlabel "daytime"
set xtics font 'Arial,9'
set xtics border nomirror out scale 1.0 ("00:00" 0, "02:00" 7200, "04:00" 14400, "06:00" 21600, "08:00" 28800, "10:00" 36000, "12:00" 43200,\
 "14:00" 50400, "16:00" 57600, "18:00" 64800, "20:00" 72000, "22:00" 79200, "24:00" 86400)
set mxtics 8
set xlabel offset 0,+0.4
plot "../$FILENAMEPREFIX_traces.txt_profile_0.txt" index 2 using 6:4 title "professor 3" w p lc 1

unset multiplot
set key nobox
unset yrange

#=======================

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
plot "../$FILENAMEPREFIX_traces.txt_profile_1.txt" index 0 using 6:4 title "phd student 1" w p lc 1
set ylabel "amount in BTC"
set ylabel offset +2.8,+5
plot "../$FILENAMEPREFIX_traces.txt_profile_1.txt" index 1 using 6:4 title "phd student 2" w p lc 1
set bmargin 2.9
set ylabel " "
set xlabel "daytime"
set xtics font 'Arial,9'
set xtics border nomirror out scale 1.0 ("00:00" 0, "02:00" 7200, "04:00" 14400, "06:00" 21600, "08:00" 28800, "10:00" 36000, "12:00" 43200,\
 "14:00" 50400, "16:00" 57600, "18:00" 64800, "20:00" 72000, "22:00" 79200, "24:00" 86400)
set mxtics 8
set xlabel offset 0,+0.4
plot "../$FILENAMEPREFIX_traces.txt_profile_1.txt" index 2 using 6:4 title "phd student 3" w p lc 1

unset multiplot
set key nobox
unset yrange

#=======================

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
plot "../$FILENAMEPREFIX_traces.txt_profile_2.txt" index 0 using 6:4 title "student 1" w p lc 1
set ylabel "amount in BTC"
set ylabel offset +2.8,+5
plot "../$FILENAMEPREFIX_traces.txt_profile_2.txt" index 1 using 6:4 title "student 2" w p lc 1
set bmargin 2.9
set ylabel " "
set xlabel "daytime"
set xtics font 'Arial,9'
set xtics border nomirror out scale 1.0 ("00:00" 0, "02:00" 7200, "04:00" 14400, "06:00" 21600, "08:00" 28800, "10:00" 36000, "12:00" 43200,\
 "14:00" 50400, "16:00" 57600, "18:00" 64800, "20:00" 72000, "22:00" 79200, "24:00" 86400)
set mxtics 8
set xlabel offset 0,+0.4
plot "../$FILENAMEPREFIX_traces.txt_profile_2.txt" index 2 using 6:4 title "student 3" w p lc 1

unset multiplot
set key nobox
unset yrange

#=======================

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
plot "../$FILENAMEPREFIX_traces.txt" using 6:4 title "all users" w p lc 1
plot "../$FILENAMEPREFIX_traces.txt" using ((int($2)==0)? $6 : -1000):4 title "professors" w p lc 2
set ylabel "amount in BTC"
set ylabel offset +2.8,+5
plot "../$FILENAMEPREFIX_traces.txt" using ((int($2)==1)? $6 : -1000):4 title "phd students" w p lc 3
set bmargin 2.9
set ylabel " "
set xlabel "daytime"
set xtics font 'Arial,9'
set xtics border nomirror out scale 1.0 ("00:00" 0, "02:00" 7200, "04:00" 14400, "06:00" 21600, "08:00" 28800, "10:00" 36000, "12:00" 43200,\
 "14:00" 50400, "16:00" 57600, "18:00" 64800, "20:00" 72000, "22:00" 79200, "24:00" 86400)
set mxtics 8
set xlabel offset 0,+0.4
plot "../$FILENAMEPREFIX_traces.txt" using ((int($2)==2)? $6 : -1000):4 title "students" w p lc 4

unset multiplot
set key nobox
unset yrange

#=======================

numberofprofessors = `grep '^.*professor.*[0-9]\{1,\}$' ../$FILENAMEPREFIX_statistics.txt | grep -oPe "[0-9]+"`
numberofphdstudents = `grep '^.*phdstudent.*[0-9]\{1,\}$' ../$FILENAMEPREFIX_statistics.txt | grep -oPe "[0-9]+"`
numberofstudents = `grep '^.* student.*[0-9]\{1,\}$' ../$FILENAMEPREFIX_statistics.txt | grep -oPe "[0-9]+"`
numberofusers = numberofprofessors+numberofphdstudents+numberofstudents

#=======================
set terminal png size 800, 800
set datafile separator ","
set output "plot_traces_overall_time.png"
unset title
set tmargin 0
set bmargin 0.8
set rmargin 1.6
set lmargin 9
set multiplot layout 4, 1 #title "Number of Transactions issued by users (no stores, etc.)"


#set xdata time
#set timefmt "%H:%M:%S"
#set format x "%H:%M"

#set xrange [ 0 : 20 ]
#set yrange [ 0 : 2 ]
#set mytics 5
#set xtics ("00:00" 0, "02:00" 7200, "04:00" 14400, "06:00" 21600)

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
plot "../$FILENAMEPREFIX_traces.txt" using (bin($6,binwidth)):(1.0) smooth freq with boxes title "all users" lt 1
plot "../$FILENAMEPREFIX_traces.txt" using ((int($2)==0)? (bin($6,binwidth)) : -1000.0):(1.0) smooth freq with boxes title "professors" lt 2
plot "../$FILENAMEPREFIX_traces.txt" using ((int($2)==1)? (bin($6,binwidth)) : -1000.0):(1.0) smooth freq with boxes title "phd students" lt 3
plot "../$FILENAMEPREFIX_traces.txt" using ((int($2)==2)? (bin($6,binwidth)) : -1000.0):(1.0) smooth freq with boxes title "students" lt 4
unset table

set datafile separator whitespace

set key inside right
set xtics border nomirror out scale 0.0 ("" 0, "" 7200, "" 14400, "" 21600, "" 28800, "" 36000, "" 43200,\
 "" 50400, "" 57600, "" 64800, "" 72000, "" 79200, "" 86400)
set ylabel " "
set tmargin 1
plot "overall_time_plot.txt" index 0 using 1:($2/numberofusers) with boxes title "all users" lt 1
set tmargin 0
plot "overall_time_plot.txt" index 1 using 1:($2/numberofprofessors) with boxes title "professors" lt 2
set ylabel "avg. number of transactions"
set ylabel offset +1.5,+4
plot "overall_time_plot.txt" index 2 using 1:($2/numberofphdstudents) with boxes title "phd students" lt 3
set bmargin 2.9
set xtics border nomirror out scale 1.0 ("00:00" 0, "02:00" 7200, "04:00" 14400, "06:00" 21600, "08:00" 28800, "10:00" 36000, "12:00" 43200,\
 "14:00" 50400, "16:00" 57600, "18:00" 64800, "20:00" 72000, "22:00" 79200, "24:00" 86400)
set xtics font 'Arial,9'
set mxtics 8
set ylabel " "
set xlabel "daytime (window size: 15min)"
set xlabel offset 0,+0.4
plot "overall_time_plot.txt" index 3 using 1:($2/numberofstudents) with boxes title "students" lt 4

unset multiplot
set datafile separator ","

#=======================
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
#set logscale y 10
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
plot "../$FILENAMEPREFIX_traces.txt" using (bin($4,binwidth)):(1.0) smooth freq with boxes title "all users" lt 1
plot "../$FILENAMEPREFIX_traces.txt" using ((int($2)==0)? (bin($4,binwidth)) : -1000.0):(1.0) smooth freq with boxes title "professors" lt 2
plot "../$FILENAMEPREFIX_traces.txt" using ((int($2)==1)? (bin($4,binwidth)) : -1000.0):(1.0) smooth freq with boxes title "phd students" lt 3
plot "../$FILENAMEPREFIX_traces.txt" using ((int($2)==2)? (bin($4,binwidth)) : -1000.0):(1.0) smooth freq with boxes title "students" lt 4

unset table

set datafile separator whitespace

set multiplot layout 4, 1 #title "Number of Transactions issued by users (no stores, etc.)"
set logscale y
set tmargin 1
set xtics border nomirror out scale 0.0 binwidth*5
set xtics font 'Arial,0'
plot 'overall_amount_plot.txt' index 0 using 1:($2/numberofusers) with boxes title "all users" lt 1
set tmargin 0
plot 'overall_amount_plot.txt' index 1 using 1:($2/numberofprofessors) with boxes title "professors" lt 2
set ylabel "avg. number of transactions (log scale)"
set ylabel offset +2.8,+5
plot 'overall_amount_plot.txt' index 2 using 1:($2/numberofphdstudents) with boxes title "phd students" lt 3
set bmargin 2.9
set xtics border nomirror out scale 1.0 binwidth*5
set xtics font 'Arial,9'
set mxtics 5
set ylabel " "
set xlabel "amount (window size: 10 BTC)"
set xlabel offset 0,+0.4
plot 'overall_amount_plot.txt' index 3 using 1:($2/numberofstudents) with boxes title "students" lt 4
unset multiplot



