<?php
require ('sqlite.database.php');

$db = new CpmSQLiteDatabase('power.sqlite3');
$result = $db->getData('minute');
$db = null;

$data = array('schema' => array(
                              array('name' => 'datetime', 'type' => 'string'),
                              array('name' => 'watthour', 'type' => 'float')
                          ), 
              'data' => array()
             );
             
for ($i=0; $i < count($result['date']); $i++) {
   $data["data"][] = array('datatime' => $result['date'][$i], 'watthour' => $result['watt'][$i] );
}
           
header('Content-type: application/json');
echo json_encode($data);
?>