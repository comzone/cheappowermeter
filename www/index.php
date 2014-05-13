<?php
require ('sqlite.database.php');
// Standard inclusions
include("pChart/pData.class.php");
include("pChart/pDraw.class.php");
include("pChart/pImage.class.php");

$currency = "kr.";
$kwhprice = "2.1";
$location = "Home";
$interval = isset($_REQUEST['interval']) ? $_REQUEST['interval'] : 'hour';

$db = new CpmSQLiteDatabase('power.sqlite3');
$result = $db->getData($interval);
$db = null;

$intervalmultiply = 1;


// Dataset definition
$dataset = new pData();

$dataset->addPoints(array_reverse($result['watt']),"Usage");
$dataset->setSerieDescription("Usage","Usage");

$dataset->setAxisName(0,"Usage");
$dataset->setAxisUnit(0,"kWh");

// Initialise the graph
$graph = new pImage(800,350, $dataset);
$graph->setFontProperties("Fonts/tahoma.ttf",10);
$graph->setGraphArea(60,30,680,290);

// Finish the graph]
$graph->setFontProperties(array("FontName"=>"Fonts/tahoma.ttf","FontSize"=>9));
$scaleSettings = array("XMargin"=>10,"YMargin"=>10,"Floating"=>False,"GridR"=>200,"GridG"=>200,"GridB"=>200,"DrawSubTicks"=>TRUE,"CycleBackground"=>TRUE,"LabelSkip"=>3);
$graph->drawScale($scaleSettings);
$graph->setFontProperties(array("FontName"=>"Fonts/tahoma.ttf","FontSize"=>9));

$graph->drawSplineChart(array("DisplayValues"=>FALSE,"DisplayColor"=>DISPLAY_AUTO));
$graph->Render("tmp/chart.png");

?>
<!DOCTYPE html>
<html>
<head>
	<title>CheapPowerMeter</title>
   <link rel="stylesheet" type="text/css" href="style.css">
</head>
<body>
   <header>
	   <h1>CheapPowerMeter</h1>
   </header>
   <div id="application">
   	<nav>
   		<ul>
   			<li><a href='?interval=hour'>Hours</a></li>
   			<li><a href='?interval=day'>Days</a></li>
   			<li><a href='?interval=month'>Months</a></li>
   		</ul>
   	</nav>
   	Usage right now: <strong><?php echo round(($result['watt'][0]*$intervalmultiply),2) ?> kWh / <?php echo round(((($result['watt'][0]*$intervalmultiply))*$kwhprice),2) . " $currency p/hr." ?></strong>
	
   	<img src="tmp/chart.png" alt="" class="chart"/>
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
   	         <td style="width:100px;"><?php echo round($value,3) ?> kWh</td>
   	         <td style="width:100px;"><?php echo round(((($value)*$kwhprice)),4) . " $currency" ?> </td>
   	      </tr>
   	      <?php } ?>
   	   </tbody>
   	</table>
   </div>
</body>
</html>