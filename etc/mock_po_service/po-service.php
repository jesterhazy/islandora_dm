<?php 
  $xml = file_get_contents('example-response.xml');
  
  $xml = str_replace('<poNumber>example</poNumber>', '<poNumber>' . substr($_SERVER['PATH_INFO'], 1) . '</poNumber>', $xml);
  echo $xml;
?>