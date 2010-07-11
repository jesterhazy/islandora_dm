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
      watchdog(t('islandora_dm'), t('ignoring invalid call to Search::execute'), null, WATCHDOG_WARNING);
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
    
    // TODO foreach on a field list
    $query = 'islandora-dm.po:(' . $escaped . ')';
    
    // TODO placeholder for document fields
    $query .= 'or islandora-dm.po.collection:(' . $escaped . ')';
    
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
        switch ($doc->{'islandora-dm.po.cmodel'}) {
          case 'islandora-dm:po-cmodel':
            $pids[] = $doc->PID;
            break;
          case 'islandora-dm:po-document-cmodel':
            $pids[] = $doc->{'islandora-dm.po.collection'};
            break;
          default:
            // skip result 
            // TODO log error on unexpected cmodel (and fix query)
            break;
        }
      }
    }
    
    return array_unique($pids);
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