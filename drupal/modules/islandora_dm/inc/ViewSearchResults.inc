<?php

/**
 * Builds the search results view.
 */
class ViewSearchResults {
	
	function __construct($results) {
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
	
	function fudgeResults() {
		// hack results		
		
		$results = array();
		
		$fake = new stdClass();
		$fake->PID = 'islandora-dm:n-example-1';
		$fake->{'rels.hasModel'} = Constants::PID_CMODEL_NOTE;
		$results[] = $fake;
		
		$fake = new stdClass();
		$fake->PID = 'islandora-dm:n-example-2';
		$fake->{'rels.hasModel'} = Constants::PID_CMODEL_NOTE;
		$results[] = $fake;
		
		$fake = new stdClass();
		$fake->PID = 'islandora-dm:p-example-2';
		$fake->{'rels.hasModel'} = Constants::PID_CMODEL_PAGE;
		$results[] = $fake;
		
		$fake = new stdClass();
		$fake->PID = 'islandora-dm:d-example-1';
		$fake->{'rels.hasModel'} = Constants::PID_CMODEL_DOCUMENT;
		$results[] = $fake;
		
		$fake = new stdClass();
		$fake->PID = 'islandora-dm:po-example-1';
		$fake->{'rels.hasModel'} = Constants::PID_CMODEL_PURCHASE_ORDER;
		$results[] = $fake;
		
		$this->results->response->numFound = count($results);
	
		$this->results->response->docs = $results;
	}
	
	function view() {
		drupal_add_js(drupal_get_path('module', Constants::MODULE_NAME) . '/js/search.js');
		$html = '<h3>' . t('Search Results') . '</h3>';
		$html .= '<div class="islandora-dm-search-results">';
		
		// $this->fudgeResults();
		
		if ($this->hits() > 0) {
		  // $view .= $this->getSearchResultsSummary();
		  foreach ($this->results->response->docs as $result) {
				$html .= $this->renderResult($result);
		  }
		}
		
		else {
			$html .= '<p>' . t('No results.') . '</p>';
		}
		
		$html .= '</div>';
		
		return $html;
	}
}