ALTER TABLE watthours RENAME TO _watthours;

CREATE TABLE IF NOT EXISTS watthours ( 
			id INTEGER PRIMARY KEY AUTOINCREMENT, 
			datetime DATETIME DEFAULT CURRENT_TIMESTAMP,
			watthour INTEGER NOT NULL);

INSERT INTO watthours (datetime, watthour)
SELECT STRFTIME('%Y-%m-%d %H:%M', datetime) as date, count(*)
FROM _watthours 
GROUP BY date 
ORDER BY date ASC;

DROP TABLE _watthours;