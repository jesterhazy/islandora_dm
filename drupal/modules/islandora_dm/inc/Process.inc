<?php

class Process {
  const WARN = 'warn';
  const ERROR = 'error';
  const STATUS = 'status';
  const MSG_KILL_ERR = 'Normal shutdown failed. Using SIGKILL.';
  const MSG_SIGKILL_ERR = 'Shutdown failed (even SIGKILL) for !name with PID %pid.';
  const MSG_STARTED = 'Started !name with PID %pid.';
  const MSG_START_ERR = '!name startup failed.';
  const MSG_STOPPED = 'Stopped !name with PID %pid.';
  const MSG_NOT_RUNNING = '!name not running under PID %pid.';
  
  protected $name;
  protected $cwd;
  protected $cmd;
  protected $log;
  protected $env;
  protected $pid;
  
  function pid() {
    return $this->pid;
  }

  function start() {
    if ($this->cmd) {
      $exec_cmd = $this->cwd ? "cd $this->cwd; " : '';
      $exec_cmd .= "nohup ./$this->cmd > $this->log 2>&1 & echo \$!";
      
      foreach($this->env as $key => $value) {
        putenv("$key=$value");
      }
      
      $pid = trim(exec($exec_cmd, $output));
    }
    
    if ($pid > 0) {
      $this->pid = $pid;
    }
    
    if ($this->isRunning()) {
      $this->message(self::MSG_STARTED);
    }
     
    else {
      $this->message(self::MSG_START_ERR, self::ERROR);
    }
  }
  
  function stop () {
    if ($this->pid) {
      if ($this->isRunning()) {
         exec("pkill -P $this->pid; sleep 1");

         if ($this->isRunning()) {
           $this->message(self::MSG_KILL_ERR, self::WARN);
           exec("pkill -9 -P $this->pid; sleep 1");
         }
         
         if ($this->isRunning()) {
           $this->message(self::MSG_SIGKILL_ERR, self::ERROR);
         }
         
         if (!$this->isRunning()) {
           $this->message(self::MSG_STOPPED);
         }
      }
      
      else {
        $this->message(self::MSG_NOT_RUNNING, self::WARN);
      }
    }
  }
  
  function isRunning() {
    $running = false;
    if ($this->pid > 0) {
      exec("ps $this->pid", $output);
      $running = (count($output) >= 2); 
    }
    
    return $running;
  }
  
  function message($msg, $level = self::STATUS) {
    $args = array(
      '!name' => $this->name,
      '%pid' => $this->pid,
    );
    
    drupal_set_message(t($msg, $args), $level);
  }
  
  /**
   * Create a Process object.
   */
  static function create($args) {
    
    $defaults = array( 
      'name' => 'Process',
      'log' => '/dev/null',
      'env' => array(),
    );
    
    $args = $args + $defaults;
    $p = new Process();
    
    $p->pid = $args['pid'];
    $p->name = $args['name'];
    $p->cmd = $args['cmd'];
    $p->cwd = $args['cwd'];
    $p->env = $args['env'];
    $p->log = $args['log'];
    
    return $p;
  }
}
