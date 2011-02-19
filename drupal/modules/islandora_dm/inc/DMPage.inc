<?php

/*
 * DMPage - methods for retrieving, displaying, and manipulating pages.
 */
class DMPage extends DMObject {
	const REIMPORT_LOG_MESSAGE = 'Islandora DM: reschedule import';
	
	function typeLabel() {
		return 'Page';
	}
	
	function pidPrefix() {
		return 'p-';
	}
	
	function createFedoraObject() {
		parent::createFedoraObject();
		$this->addModel(Constants::PID_CMODEL_PAGE, FEDORA_MODEL_URI);
	}
	
	function addTiffFile($file) {
		// create image datastream
		$this->addDatastreamFromFile($file->filename, Constants::DSID_TIFF, Constants::DSLABEL_TIFF, Constants::MIME_TYPE_TIFF);
	}
	
	function document() {
		return new DMDocument(DMObject::findDocumentPid($this));
	}
	
	function incompleteImport() {
		return $this->isMember(Constants::PID_COLLECTION_PURCHASE_ORDERS_INCOMPLETE_IMPORT);
	}
	
	function thumbnailUrl() {
		$url = NULL;
		$ds = $this->getDatastream(Constants::DSID_TN);
		
		if ($ds) {
			global $base_url;
			$url = $base_url . '/fedora/repository/' . $this->pid . '/tn';
		}
		
		return $url;
	}

        function viewerLinkUrl() {
		return $this->document()->viewerLinkUrl();
        }

	static function findRelatedPages($parent_pid) {
		$pages = array();
		
		$pids = DMObject::findRelatedPids($parent_pid, Constants::PID_CMODEL_PAGE);
		foreach($pids as $pid) {
			$pages[] = new DMPage($pid);
		}
		
		return $pages;
	}
	
	function rescheduleImport() {
		// adds a log message to fedora
		// this triggers an api-m jms update (modifyObject)
		// that the fedora_microservices listener can detect
		$this->addLogMessage(self::REIMPORT_LOG_MESSAGE);
	}
}
