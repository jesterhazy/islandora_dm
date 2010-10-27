<?php

abstract class DMObject {
  public $fedora;
  
  function pid() {
    return $this->fedora ? $this->fedora->pid : NULL;
  }
  
  function generatePid() {
    return generate_pid();
  }
  
  function typeLabel() {
    return 'Object';
  }
  
  function stripNamespace($pid) {
    return substr($pid, strlen(Constants::PID_NAMESPACE) + 1);
  }
  
  function generateTitle($pid) {
    return $this->typeLabel() . ' ' . $this->stripNamespace($pid);
  }
  
  function createFedoraItem() {
    $pid = $this->generatePid();
    $title = $this->generateTitle($pid);
    
    $this->fedora = create_fedora_item_with_dc($pid, $title);
  }

  function joinCollection($pid) {
    $pids = is_array($pid) ? $pid : array($pid);
    
    foreach($pids as $pid) {
      $this->fedora->add_relationship('isMemberOf', $pid);
    }
  }
}
