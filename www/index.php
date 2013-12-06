<?php
require ('sqlite.database.php');

$currency = "kr.";
$kwhprice = "2.1";
$location = "Home";
$interval = isset($_REQUEST['interval']) ? $_REQUEST['interval'] : 'hour';

$db = new CpmSQLiteDatabase('power.sqlite3');
$result = $db->getData($interval);
$db = null;

$intervalmultiply = 1;

echo "<a href='?interval=minute'>Minutes</a> <a href='?interval=hour'>Hours</a> <a href='?interval=day'>Days</a> <a href='?interval=month'>Months</a> <br><br>";
echo "Usage right now: <b>".round(($result['watt'][0]*$intervalmultiply),2)." kWh / ".round(((($result['watt'][0]*$intervalmultiply))*$kwhprice),2)." $currency p/hr.</b><br>";

// Standard inclusions
include("pChart/pData.class");
include("pChart/pChart.class");

// Dataset definition
$DataSet = new pData;


$DataSet->AddPoint($result['watt']);
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
$Test->Render("tmp/chart.png");

echo "<img src=tmp/chart.png><br><br>";
?>
<table>
   <thead>
      <tr>
         <th align="left">Date</th>
         <th align="left">kWh</th>
         <th align="left">Price</th>
      </tr>
   </thead>
   <tbody>
      <?php foreach ($result['watt'] as $key => $value) { ?>
      <tr>
         <td style="width:150px;"><?php echo $result['date'][$key] ?></td>
         <td style="width:100px;"><?php echo round($value,2) ?> kWh</td>
         <td style="width:100px;"><?php echo round(((($value)*$kwhprice)),2) . " $currency" ?> </td>
      </tr>
      <?php } ?>
   </tbody>
</table>