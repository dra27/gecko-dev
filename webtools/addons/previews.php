<?php
/**
 * Addon previews page.  Displays all the previews for a particular addon or extension.
 * 
 * @package amo
 * @subpackage docs
 */
 
// Arrays to store clean inputs.
$clean = array();  // General array for verified inputs.
$sql = array();  // Trusted for SQL.

// Get our addon ID.
$clean['ID'] = intval($_GET['id']);
$sql['ID'] =& $clean['ID'];

$addon = new AddOn($sql['ID']);
$addon->getPreviews();

// Assign template variables.
$tpl->assign(
    array(  'addon'     => $addon,
            'title'     => $addon->Name.' Previews &amp; Screenshots',
            'content'   => 'previews.tpl',
            'sidebar'   => 'inc/addon-sidebar.tpl')
);
