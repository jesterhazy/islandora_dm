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
  protected $url;
  
  function __construct($offset, $limit) {
    $this->offset = $offset;
    $this->limit = $limit;
  }
  
  /**
   * Execute a term search, and return a Search instance containing the results.
   */
  static function createSearchUsingTerms($terms, $offset = 0, $limit = 10) {
    $search = new Search($offset, $limit);
    $search->setQueryFromTerms($terms);
    return $search;
  }
  
  /**
   * Set query from search terms.
   */
  function setQueryFromTerms($terms) {    
    // split terms    
    $split = split(' ', $terms);
    $joined = join(" and ", $split);
    $escaped = Apache_Solr_Service::escape($joined);
    $query = 'islandora-dm.po:(' . $escaped . ')';
    $this->query = $query;
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
   * Prepare a solr service instance for use.
   */
  function getSolrService() {
    $solr_url = variable_get(Constants::SETTING_SOLR_URL, Constants::SETTING_SOLR_URL_DEFAULT);
    $parts = parse_url($solr_url);
    
    $solr = new Apache_Solr_Service($parts['host'], $parts['port'], $parts['path']);
    return $solr;
  }
  
  function results() {
    return $this->results;
  }
  
  
  
  
  
   // 
   // 
   // function getSearchResultsSummary() {
   //   $hits = $this->hits();
   //   $html .= '<div class="islandora-dm-search-results-summary">';
   //   
   //   if ($hits == 0) {
   //     $html .= t('Nothing found.');
   //   }
   //  
   //   elseif ($hits <= $this->limit) {
   //     $html .= t('Showing all results.');
   //   }
   //   
   //   else {
   //     $end = min($this->offset + $this->limit, $hits);
   //     $html .= t('Showing @start - @end of @hits results.', 
   //       array('@start' => $this->offset + 1, '@end' => $end, '@hits' => $hits));
   //   }
   //   
   //       
   //   if ($hits > $this->limit) {
   //     $html .= '<div class="islandora-dm-search-results-pager">';
   //     
   //     $url = $this->url;
   //     $url .= (strstr($url, '?')) ? '&' : '?';
   //     $url .= 'start=';
   // 
   //     
   //     if ($this->offset > $this->limit) {
   //       $html .= '<a href="' . $url . '0">&lt;</a>';
   //     }
   //     
   //     $offset = 0;
   //     $ellipse1 = FALSE;
   //     $ellipse2 = FALSE;
   //     while($offset < $hits) {
   //       $page = (int) ($offset / $this->limit) + 1;
   //       if ($offset == 1 || $offset + $this->limit >= $hits || abs($offset - $this->offset) < (3 * $this->limit)) {
   //         $html .= ' <a href="' . $url . $offset . '">' . $page . '</a>';
   //       }
   //       
   //       elseif(!$ellipse1 && $offset - $this->offset <= -3) {
   //         $html .= ' ...';
   //         $ellipse1 = TRUE;
   //       }
   //       
   //       elseif(!$ellipse2 && $offset - $this->offset >= 3) {
   //         $html .= ' ...';
   //         $ellipse2 = TRUE;
   //       }
   // 
   //       $offset += $this->limit;
   //     }
   //     
   //     if ($this->offset + $this->limit < $hits) {
   //       $html .= ' <a href="' . $url . $last_page . '">&gt;</a>';
   //     }
   //     
   //     $html .= '</div>';
   //   }
   //   
   //   $html .= '</div>';
   //   return $html;
   // }


}
