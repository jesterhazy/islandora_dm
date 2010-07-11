<?php

class JMSProcess {
  const PROCESS_NAME = 'JMS service';
  
  static function create($args = array()) {
    $defaults = array('name' => self::PROCESS_NAME);
    $args = $args + $defaults;
    
    return Process::create($args);
  }
  
  static function createExisting() {
    $pid = escapeshellcmd(variable_get('islandora_dm_java_jms_pid', null));
    return self::create(array('pid' => $pid));
  }
  
  static function createWithCmd($args = array()) {
    $defaults = array(
      'cmd' => 'start.sh',
      'cwd' => trim(`pwd`) . '/' . drupal_get_path('module', 'islandora_dm') . '/etc/java/bin',
      'output' => escapeshellcmd(variable_get('islandora_dm_java_jms_logfile', null)),
      'env' => array(
         '_islandora_dm_java_fedora_url' => variable_get('islandora_dm_java_fedora_url', ''),
         '_islandora_dm_java_fedora_user' => variable_get('islandora_dm_java_fedora_user', ''),
         '_islandora_dm_java_fedora_pass' => variable_get('islandora_dm_java_fedora_pass', ''),
       ),
    );
    $args = $args + $defaults;
    
    return self::create($args);
  }
  
  static function start() {
    $p = self::createWithCmd();
    $p->start();
    if ($p->isRunning()) {
      variable_set('islandora_dm_java_jms_pid', $p->pid());
    }
    
    return $p;
  }
}