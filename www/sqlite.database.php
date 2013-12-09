<?php
require_once('database.php');

class CpmSQLiteDatabase implements CpmDatabase {

   protected $db;
   private $interval = array('minute' => array('format' => '%Y-%m-%d %H:%M', 'limit' => 60),
                                               'hour' => array('format' => '%Y-%m-%d %H', 'limit' => 24),
                                               'day' => array('format' => '%Y-%m-%d', 'limit' => 31),
                                               'month' => array('format' => '%Y-%m', 'limit' => 12)
   );

   public function __construct($sqlitefile) {

      if (empty($sqlitefile)) {
         throw new \Exception("Missing database file");
      }
      try {
         $this->db = new \PDO('sqlite:' . $sqlitefile);
         $this->db->setAttribute(\PDO::ATTR_ERRMODE, \PDO::ERRMODE_EXCEPTION);
      } catch (PDOException $e) {
         echo $e->getMessage();
         die;
      }
   }

   public function __destruct() {
      $this->db = null;
   }

   public function getData($interval) {

      $results = array('watt' => array(),
                       'date' => array()
      );

      if (!array_key_exists($interval, $this->interval)) {
         throw new \Exception("Bad interval");
      }

      $query = "SELECT STRFTIME(:format, datetime, 'localtime') as date, count(id) as watt FROM watthours group by date order by date desc limit :limit";
      $stmt = $this->db->prepare($query);

      $stmt->bindParam(':format', $this->interval[$interval]['format'], \PDO::PARAM_STR);
      $stmt->bindParam(':limit', $this->interval[$interval]['limit'], \PDO::PARAM_INT);

      $stmt->execute();
      $tmpResult = $stmt->fetchAll(\PDO::FETCH_ASSOC);

      for ($i = 0; $i < count($tmpResult); $i++) {
         $results['watt'][$i] = floatval($tmpResult[$i]['watt'] / 1000);
         $results['date'][$i] = $tmpResult[$i]['date'];
      }

      return $results;
   }

}

?>