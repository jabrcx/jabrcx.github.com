plots:
	logstats --count 'Data-Mover Calls: data-mover called' --save-plots  data-mover.log
	logstats --count 'Get Calls: data-get' --count 'Put Calls: data-put' --save-plots  data-mover.log
	logstats --count 'Token Acquisitions: i/o token acquired' --breakdown-by-field 10 --save-plots  data-mover.log
	logstats --count 'Token Acquisitions: i/o token acquired' --breakdown-by-field 10 --totals-only --save-plots  data-mover.log
	convert 120219070000-120219202959.count.Token_Acquisitions.breakdown10.total.png -resize 75% 120219070000-120219202959.count.Token_Acquisitions.breakdown10.total.smaller.png
	logstats --sum 'Data Downloaded: data-get --size (\d+)' --units bytes --save-plots  data-mover.log
	logstats --sum 'Data Downloaded: data-get --size (\d+)' --cumulative --units TB --units-conversion-factor bytes-to-TB --save-plots  data-mover.log
	logstats --average 'Data Downloaded: data-get --size (\d+)' --units bytes --save-plots  data-mover.log
	logstats --average 'Data Downloaded: data-get --size (\d+)' --units bytes --tstart '2012-02-19 16:30:00' --tend '2012-02-19 16:45:00' -t 60 --save-plots  data-mover.log
