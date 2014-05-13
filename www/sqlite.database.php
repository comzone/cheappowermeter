<?php
require_once('database.php');

class CpmSQLiteDatabase implements CpmDatabase {

   protected $db;
   private $interval = array('hour'    => array('format' => '%Y-%m-%d %H'     , 'limit' => 24),
                             'day'     => array('format' => '%Y-%m-%d'        , 'limit' => 744),
                             'month'   => array('format' => '%Y-%m'           , 'limit' => 8760)
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

      $query = "SELECT STRFTIME(:format, datetime, 'localtime') AS date, sum(watthour) AS watt 
                FROM watthours 
                WHERE datetime > DATETIME('now', :limit) 
                GROUP BY date 
                ORDER BY date DESC";
                
      $stmt = $this->db->prepare($query);

      $limit = '-' . $this->interval[$interval]['limit'] . ' HOUR';
      $stmt->bindParam(':format', $this->interval[$interval]['format'], \PDO::PARAM_STR);
      $stmt->bindParam(':limit', $limit, \PDO::PARAM_STR);

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