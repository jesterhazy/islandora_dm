<?php

/**
 * Files - helper methods for dealing with files and directory
 */
class Files {
  const FILENAME_PATTERN_ALL = '^.*$';
  const FILE_SIZE_UNIT_LABELS = 'bytes, KB, MB, GB';
  const DATE_FORMAT_MINUTE_PRECISION = '%Y-%m-%d at %H:%M';
  
  static function listFiles($dir, $pattern = self::FILENAME_PATTERN_ALL, $addMetadata = true) {
    $files = file_scan_directory($dir, $pattern);
    
    if ($addMetadata) {
      self::addMetadata($files);
    }
    
    return $files;
  }
  
  static function addMetadata($files) {
    foreach ($files as $file) {
      $file->size = filesize($file->filename);
      $file->mtime = filemtime($file->filename);
    }
  }
  
  static function formatFilesize($size) {
    $labels = explode(', ', self::FILE_SIZE_UNIT_LABELS);
    $level = 0;
    $remainder = 0;
        
     while ($size > 1024 && count($labels) > $level + 1) {
           $remainder = $size % 1024;
           $size = intval($size / 1024);
           $level++;
         }
    
    return $size . ($remainder > 0 ? '.' . $remainder : '') . ' ' . $labels[$level];
  }
  
  static function extractBasenames($files) {
    return array_map(array('Files', 'extractBaseName'), $files);
  }
  
  static function extractBasename($file) {
    return $file->basename;
  }
  
  static function formatFilemtime($time) {
    return strftime(self::DATE_FORMAT_MINUTE_PRECISION, $time);
  }
}