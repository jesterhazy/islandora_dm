<?php

/**
 * Builds the search results view.
 */
class ViewSearchResults {
	
	function __construct($results, $offset) {
		$this->offset = $offset;
		$this->results = $results;
	}
	
	function hits() {
		$n = 0;
		
		if ($this->results) {
			$n = $this->results->response->numFound;
		}
		
		return $n;
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

	/** TODO rewrite for new content model */
	function resultToPid(&$doc, &$pids) {
		if (Constants::PID_CMODEL_PURCHASE_ORDER == $doc->{'rels.hasModel'}) {
			$pids[] = $doc->PID;
		}

		elseif (strpos($doc->{'rels.hasModel'}, 'islandora-dm:') !== FALSE) {
			$collections = is_array($doc->{'rels.isMemberOf'}) 
				? $doc->{'rels.isMemberOf'} 
					: array($doc->{'rels.isMemberOf'});

			foreach ($collections as $collection) {
				if (DMPurchaseOrder::isPurchaseOrderPid($collection)) {
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
	
	function renderResult($result) {
		$html = '';
		
		$pid = $result->PID;
		$cmodel = $result->{'rels.hasModel'};	
		
		switch($cmodel) {
			case Constants::PID_CMODEL_PURCHASE_ORDER:
				$html .= $this->renderPurchaseOrder($pid);
				break;
			case Constants::PID_CMODEL_DOCUMENT:
				$html .= $this->renderDocument($pid);
				break;
			case Constants::PID_CMODEL_PAGE:
				$html .= $this->renderPage($pid);
				break;
			case Constants::PID_CMODEL_NOTE:
				$html .= $this->renderNote($pid);
				break;
			default:
				watchdog(Constants::MODULE_NAME, 
					'unexpected content model in search results. pid: @pid, cmodel: @cmodel', 
					array('@pid' => $pid, '@cmodel' => $cmodel), 
					WATCHDOG_WARN);
				break;
		}
		
		return $html;
	}
	
	function renderPurchaseOrder($pid) {
		$purchase_order = new DMPurchaseOrder($pid);
		$purchase_order->load();
		$view = new ViewPurchaseOrder($purchase_order);
		return $view->searchResult();
	}
	
	function renderDocument($pid) {
		$document = new DMDocument($pid);
		$document->load();
		$view = new ViewDocument($document);
		return $view->searchResult();
	}
	
	function renderPage($pid) {
		$page = new DMPage($pid);
		$page->load();
		$view = new ViewPage($page);
		return $view->searchResult();
	}
	
	function renderNote($pid) {
		$note = new DMNote($pid);
		$note->load();
		$view = new ViewNote($note);
		return $view->searchResult();
	}
	
	function view() {
		drupal_add_js(drupal_get_path('module', Constants::MODULE_NAME) . '/js/search.js');
		$html = '<h3>' . t('Search Results') . '</h3>';
		$html .= '<div class="islandora-dm-search-results">';
		
		if ($this->hits() > 0) {
		  $html .= $this->summary();
		  foreach ($this->results->response->docs as $result) {
				$html .= $this->renderResult($result);
		  }
		  $html .= $this->summary();
		}
		
		else {
			$html .= '<p>' . t('No results.') . '</p>';
		}
		
		$html .= '</div>';
		
		return $html;
	}
	
	function summary() {
		$html = '<div class="summary">';
		
		
		$hits = $this->hits();
		$offset = $this->offset;
		if ($hits > Search::MAX_RESULTS) {

			$html .= '<div class="page-previous">';
			if ($offset > 0) {
				$html .= '<a href="' 
					. Search::queryStringForOffset($offset - Search::MAX_RESULTS) 
					. '">&lt; Previous page</a>';
			}
			$html .= '&nbsp;</div>';
			
			$html .= '<div class="page-next">&nbsp;';
			if ($offset + Search::MAX_RESULTS < $hits) {
				$html .= '<a href="' 
					. Search::queryStringForOffset($offset + Search::MAX_RESULTS) 
					. '">Next page &gt;</a>';
			}
			$html .= '</div>';
			
			$html .= 'Showing ' . $this->startNumber() . ' to ' . $this->endNumber() . ' of ' . $this->hits() . '.';
		}
		
		$html .= '</div>';
		return $html;
	}

	function startNumber() {
		return $this->offset + 1;
	}
	
	function endNumber() {
		$end = $this->offset + Search::MAX_RESULTS;
		$hits = $this->hits();
		
		if ($end > $hits) {
			$end = $hits;
		}
		
		return $end;
	}
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
