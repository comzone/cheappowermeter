DROP INDEX IX_DateTime_Watthour;
ALTER TABLE watthours RENAME TO watthours_;
CREATE TABLE "watthours" ( id INTEGER PRIMARY KEY AUTOINCREMENT, datetime DATETIME DEFAULT CURRENT_TIMESTAMP,watthour INTEGER NOT NULL);
CREATE INDEX IX_DateTime_Watthour on "watthours" (datetime, watthour);

insert into watthours (datetime, watthour)
select datetime((strftime('%s', datetime) / 300) * 300, 'unixepoch') interval,
       sum(watthour) watthour
from watthours_
group by interval
order by interval;

DROP TABLE watthours_;


