<?
$currency = "kr.";
$kwhprice = "2.1";
$location = "Home";

$link = mysql_connect('localhost', 'measurepower', 'yourpasswordhere');
if (!$link) {die('Could not connect: ' . mysql_error());}
mysql_select_db('measurepower', $link);


if($_REQUEST[interval] == "m" OR $_REQUEST[interval] == ""){
// For Minute
$result = mysql_query("SELECT DATE_FORMAT(datetime, '%Y-%m-%d %H:%i') as date, count(id) as watt FROM watthours WHERE datetime <= NOW() -60 group by date order by date desc limit 60", $link);
$intervalmultiply = 60;
}
// For Hour
if($_REQUEST[interval] == "h"){
$result = mysql_query("SELECT DATE_FORMAT(datetime, '%Y-%m-%d %H') as date, count(id) as watt FROM watthours WHERE datetime <= NOW() -60 group by date order by date desc limit 24", $link);
$intervalmultiply = 1;
}
// For Day
if($_REQUEST[interval] == "d"){
$result = mysql_query("SELECT DATE_FORMAT(datetime, '%Y-%m-%d') as date, count(id) as watt FROM watthours WHERE datetime <= NOW() -60 group by date order by date desc limit 31", $link);
$intervalmultiply = 0.04166666667;
}
// For Month
if($_REQUEST[interval] == "mo"){
$result = mysql_query("SELECT DATE_FORMAT(datetime, '%Y-%m') as date, count(id) as watt FROM watthours WHERE datetime <= NOW() -60 group by date order by date desc limit 12", $link);
$intervalmultiply = 0.138888889;
}

if (!$result) {
    die('Invalid query: ' . mysql_error());
}
    while ($row=mysql_fetch_array($result)){
        //echo "".$row['date']." ".$row['watt']."<br>";
$watt[] = (($row['watt']/1000));
$dates[] = $row['date']."";

        }



echo "<a href='?interval=m'>Minutes</a> <a href='?interval=h'>Hours</a> <a href='?interval=d'>Days</a> <a href='?interval=mo'>Months</a> <br><br>";
echo "Usage right now: <b>".round(($watt[0]*$intervalmultiply),2)." kWh / ".round(((($watt[0]*$intervalmultiply))*$kwhprice),2)." $currency p/hr.</b><br>";


// Standard inclusions
include("pChart/pData.class");
include("pChart/pChart.class");

// Dataset definition
$DataSet = new pData;


$DataSet->AddPoint($watt);
$DataSet->AddSerie();
$DataSet->SetSerieName("kWh","Serie1");

//$DataSet->addPoint($dates,"Labels");
//$DataSet->setSerieDescription("Labels",$dates);
//$DataSet->setAbscissa("Labels");

// Initialise the graph
$Test = new pChart(800,400);
$Test->setFontProperties("Fonts/tahoma.ttf",10);
$Test->setGraphArea(60,30,680,200);
$Test->drawGraphArea(252,252,252);
$Test->drawScale($DataSet->GetData(),$DataSet->GetDataDescription(),SCALE_NORMAL,150,150,150,TRUE,0,2);
$Test->drawGrid(4,TRUE,230,230,230,255);

// Draw the line graph
$Test->drawLineGraph($DataSet->GetData(),$DataSet->GetDataDescription());
$Test->drawPlotGraph($DataSet->GetData(),$DataSet->GetDataDescription(),3,2,255,255,255);

// Finish the graph
$Test->setFontProperties("Fonts/tahoma.ttf",8);
$Test->drawLegend(45,35,$DataSet->GetDataDescription(),255,255,255);
$Test->setFontProperties("Fonts/tahoma.ttf",10);
$Test->drawTitle(80,22,"kWh usage at $location",50,50,50,585);
$Test->Render("chart.png");

echo "<img src=chart.png><br><br>";

foreach ($watt as $key => $value){
echo $dates[$key]." - <b> ".round($value,2)."</b> kWh  <b>".round(((($value)*$kwhprice)),2)."</b> $currency<br>";
}

mysql_close($link);
?>
