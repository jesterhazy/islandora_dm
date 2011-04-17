<?php

class ViewPage {
	protected $page;
	
	function __construct($page) {
		$this->page = $page;
	}
	
	function searchResult() {
		
		$document = $this->page->document();
		$document->load();
		$documentView = new ViewDocument($document);

		$title = $this->page->typeLabel();
		$id = $this->page->pidNumber();
		$url = $this->page->incompleteImport() ? '#' : $this->page->linkUrl();
		return $documentView->searchResultWithTitle($title, $id, $url, $this->page->thumbnailUrl());
	}
}