<?php

module_load_include('php', 'fedora_repository', 'Solr/Service');

/**
 * Search - performs purchase order searches, and hold results.
 */ 
class Search {
  protected $query;
  protected $offset;
  protected $limit;
  protected $results;
  
  function __construct($offset, $limit) {
    $this->offset = $offset;
    $this->limit = $limit;
  }
  
  /**
   * Execute the search. Can only be called once. Does nothing on subsequent calls.
   */
  function execute() {
    
    // log warning if execute is called more than once
    if ($this->results) {
      watchdog(Constants::MODULE_NAME, t('ignoring invalid call to Search::execute'), null, WATCHDOG_WARNING);
      return;
    }
    
    $solr = $this->getSolrService();

    // TODO 'qt' param s/b from solr block config (maybe default limit, too)
    $this->results = $solr->search($this->query, $offset, $limit, array('qt' => 'standard'));
  }
  
  /**
   * Set query from search terms.
   */
  function queryTerms($terms) {
    
    // split terms    
    $split = split(' ', $terms);
    $joined = join(" and ", $split);
    $escaped = Apache_Solr_Service::escape($joined);
    $query = 'islandora-dm.po:(' . $escaped . ')';
    $this->query = $query;
  }
  
  /**
   * Set query from user.
   */
  function queryUser($user) {
    $this->setQuery('islandora-dm.po.owner', $user);
  }
  
  /**
   * Set query from status.
   */
  function queryStatus($status) {
    $this->setQuery('islandora-dm.po.status', $status);
  }
  
  protected function setQuery($field, $term) {
    // TODO if isarray (term)...
    
    if (empty($term)) {
      $term = '';
    }
    
    $this->query = $field . ':' . Apache_Solr_Service::phrase($term);
  }
  
  /**
   * Prepare a solr service instance for use.
   */
  function getSolrService() {
    // TODO add vars from islandora conf
    $solr = new Apache_Solr_Service('localhost', 8080, '/solr');
    return $solr;
  }
  
  protected function resultsToPids() {
    $pids = array();
    
    if ($this->results) {
      foreach ($this->results->response->docs as $doc) {
        $this->resultToPid($doc, $pids);
      }
    }
    
    return array_unique($pids);
  }
  
  function resultToPid(&$doc, &$pids) {
    if (Constants::PID_PO_CMODEL == $doc->{'rels.hasModel'}) {
      $pids[] = $doc->PID;
    }
    
    elseif (strpos($doc->{'rels.hasModel'}, 'islandora-dm:') !== FALSE) {
      $collections = is_array($doc->{'rels.isMemberOf'}) 
        ? $doc->{'rels.isMemberOf'} 
          : array($doc->{'rels.isMemberOf'});
      
      foreach ($collections as $collection) {
        if ($this->isPurchaseOrderPid($collection)) {
          $pids[] = $collection;
        }
      }
    }
    
    else {
      // unexpected cmodel
      watchdog(Constants::MODULE_NAME, 
        'unexpected content model in search results. pid: @pid, cmodel: @cmodel', 
        array('@pid' => $doc->PID, '@cmodel' => $doc->{'rels.hasModel'}), 
        WATCHDOG_WARN);
    }
  }
  
  function isPurchaseOrderPid($pid) {
    return preg_match('/^islandora-dm:po-[^\-]+$/', $pid);
  }
  
  /**
   * Convert the search results into a page fragment. 
   */
  function getSearchResultsView() {
    $pids = $this->resultsToPids();

    $page = '<div class="islandora-dm-search-results">';
        
    if (empty($pids)) {
      $page .= '<div class="no-results">' . t('Nothing found.') . '</div>';
    }
    
    else {
      foreach ($pids as $pid) {
        $po = PurchaseOrder::fetchByPID($pid);
        $page .= $po->getSearchResultView();
      }
    }
    
    $page .= '</div>';
    
    return $page;
  }

  /**
   * Execute a term search, and return a Search instance containing the results.
   */
  static function searchByTerms($terms, $offset = 0, $limit = 10) {
    $search = new Search($offset, $limit);
    $search->queryTerms($terms);
    $search->execute();
    return $search;
  }
  
  static function searchByUser($user, $offset = 0, $limit = 10) {
    $search = new Search($offset, $limit);
    $search->queryUser($user);
    $search->execute();
    return $search;
  }
  
  static function searchByStatus($status, $offset = 0, $limit = 10) {
    $search = new Search($offset, $limit);
    $search->queryStatus($status);
    $search->execute();
    return $search;
  }
}
