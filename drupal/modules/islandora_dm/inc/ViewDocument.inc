<?php

/**
 * Builds the note view.
 */
class ViewDocument {
	
	/**
	* The note object wrapped by this view
	*
	* @var DMDocument
	*/
	protected $document;

	function __construct($document) {
		$this->document = $document;
	}
	
	function view() {
		drupal_add_js('misc/collapse.js');

		$view = "\n\n";
		$view = '<div class="islandora-dm-view">';

		// return an empty view (with errors) if object is empty.
		// if (!$this->document->loaded()) {
		// 	return $view;
		// }

		$view .= "\n\n";
		$view .= $this->addDocumentSection();
		// $view .= '<fieldset class="collapsible collapsed"><legend>Details</legend>' . "\n";
		// $view .= '<div class="plain">';
		// $view .= theme('table', null, $this->detailsAsTableRows());
		// $view .= '</div>';
		// $view .= $this->addNotesSection();
		// $view .= '</fieldset>' . "\n";
		$view .= '<h3>Details</h3>' . "\n";
		$view .= '<div class="plain">';
		$view .= theme('table', null, $this->detailsAsTableRows());
		$view .= '</div>';
		$view .= $this->addNotesSection();
		$view .= ViewPurchaseOrder::addButton('add_note', 'Add Note');
		$view .= '</div>';		
		$view .= "\n\n";

		return $view;
	}
	
	// function addDocumentSection() {
	// 	$url = $this->document->viewerBaseUrl();
	// 	$params = $this->document->viewerUrlParameters();
	// 	$html = "\n\n";
	// 	$html .= '<fieldset class="collapsible"><legend>Document image</legend><div>';
	// 	$html .= '<a href="' . $this->document->viewerLinkUrl() . '" target="' . $this->document->pid(). '">Open in new window</a></div>';
	// 	$html .= '<iframe src="' . $this->document->viewerLinkUrl() . '" frameborder=0 style="height: 800px; width: 100%" scrolling="no">error: unable to load viewer</iframe></fieldset>';
	// 	$html .= "\n\n";
	// 	return $html;
	// }
	
	function addDocumentSection() {
		$html = '<div class="document">';
		$html .= '<div class="thumbnail">' . $this->thumbnail() . '</div>';
		$html .= '</div><div class="clear"></div>';
		return $html;
	}
	
	function addNotesSection() {
		$notes = $this->document->notes();
		return ViewNote::pageSectionView($notes);
	}
	
	function embeddedView() {
		$date = $this->document->createdDate();
		$pages = $this->document->pageCount();
	
		$view = '';
		$view .= '<div class="document">';
		$view .= '<div class="thumbnail">' . $this->thumbnail() . '</div>';
		$view .= '<div><a href="document/' . $this->document->pidNumber() . '/add_note">Add Note</a></div>';
		$view .= '<div class="caption">';
		$view .= $date . '<br>';
		$view .= $pages . ' page' . ($pages != 1 ? 's' : '');
		$view .= '</div>';
		$view .= '</div>';
		
		
		return $view;
	}
	
	function thumbnail() {
		$tag = '';

		$showViewerLink = !$this->document->incompleteImport();
		
		if ($showViewerLink) {
			$tag .= '<a href="' . $this->document->viewerLinkUrl() . '" target="' . $this->document->pid() . '">';
			$tag .= $this->thumbnailImgTag($this->document->thumbnailUrl());
			$tag .= '</a>';
		}
		
		else {
			$tag .= $this->thumbnailImgTag($this->thumbnailPlaceholderUrl());
		}
		
		return $tag;
	}
	
	function thumbnailImgTag($image_url) {
		return '<img src="' . $image_url . '" width="85" height="110" alt="' . t('document image') . '">';
	}
	
	function thumbnailPlaceholderUrl() {
		return $url = base_path() . drupal_get_path('module', Constants::MODULE_NAME)	. '/img/tn-placeholder.gif';
	}
	
	function searchResultWithTitle($title, $id, $url, $thumbnailUrl) {
		$html = '';
		
		// return an empty view (with errors) if fedora is empty.
		if (!$this->document->loaded()) {
			return $html;
		}
		
		$html .= "\n\n";
		$html .= '<div class="search-result clickable">';
		$html .= '<div class="title">' . t($title) . ' <a href="' . $url . '">' . $id . '</a></div>';
		$html .= $this->thumbnailForSearchResult($thumbnailUrl);
		$html .= theme('table', null, $this->detailsAsTableRows());
		$html .= '</div>';
		
		return $html;
	}
	
	function detailsAsTableRows() {
		$rows = array_merge($this->documentDetailsAsTableRows(), $this->purchaseOrderDetailsAsTableRows());
		return $rows;
	}
	
	function documentDetailsAsTableRows() {
		$rows = array(
			array(array('data' => t('Import Date:'), 'class' => 'label'), check_plain($this->document->createdDate()), null),
			array(array('data' => t('Pages:'), 'class' => 'label'), check_plain($this->document->pageCount()), null),
		);
		
		return $rows;
	}
	
	private function purchaseOrderDetailsAsTableRows() {
		$rows = array();
		$purchase_orders = $this->document->purchaseOrders();
		if (!empty($purchase_orders)) {
			$links = array();
			foreach($purchase_orders as $purchase_order) {
				$links[] .= '<a href="' . $purchase_order->linkUrl($purchase_order->pid()) . '">' . $purchase_order->number() . '</a>';
				
			}
			
			$rows[] = array(array('data' => t('PO #:'), 'class' => 'label'), implode($links, ', '), null);
		}
		return $rows;
	}
	
	function searchResult() {
		$title = $this->document->typeLabel();
		$id = $this->document->pidNumber();
		$url = $this->document->linkUrl();
		$thumbnailUrl = $this->document->thumbnailUrl();

		return $this->searchResultWithTitle($title, $id, $url, $thumbnailUrl);
	}

	function thumbnailForSearchResult($url) {		
		if (empty($url)) {
			$url = $this->thumbnailPlaceholderUrl();
		}
		
		$tag = $this->thumbnailImgTag($url);
		
		return '<div class="thumbnail right">' . $tag . '</div>';
	}
	
	function thumbnailForImport() {
		$tag = '<div class="thumbnail right">';
		$tag .= $this->thumbnail();
		$tag .= '</div>';
		return $tag;
	}
}
