<?php
/*---------------------------------------------------+
| mssqldump.php
+----------------------------------------------------+
| Copyright 2006 Huang Kai hkai@atutility.com
| http://atutility.com/
+----------------------------------------------------+
| Released under the terms & conditions of v2 of the
| GNU General Public License. For details refer to
| the included gpl.txt file or visit http://gnu.org
+----------------------------------------------------*/
/*
change log:
2012-11-21 James Sutherland - convert to newer SQLSRV API
2006-10-31 Huang Kai
---------------------------------
initial release

*/
$mssqldump_version="1.01";
$output_messages=array();

if( isset($_REQUEST['action']) )
{
	$db=sqlsrv_connect($_REQUEST['mssql_host'],array(
		'ReturnDatesAsStrings'	=> true,
		'UID'					=> $_REQUEST['mssql_username'],
		'PWD'					=> $_REQUEST['mssql_password'],
		'Database'				=> $_REQUEST['mssql_database']
	));
	if (!$db) {
		die(print_r(sqlsrv_errors(),true));
	}

	if( 'mssql_test' == $_REQUEST['action'])
	{
		header('Content-type: text/plain');
		echo 'OK';
		exit;
	}

	if( 'mssql_export' == $_REQUEST['action'])
	{
		if( 'SQL' == $_REQUEST['output_format'] )
		{
			//ob_start("ob_gzhandler");
			header('Content-type: text/plain');
			header('Content-Disposition: attachment; filename="'.$_REQUEST['mssql_host']."_".$_REQUEST['mssql_database']."_".date('YmdHis').'.sql"');
			echo "/*mssqldump.php version $mssqldump_version */\n";
			_mssqldump($db);

			//header("Content-Length: ".ob_get_length());

			//ob_end_flush();
			exit;
		}
		else if( 'CSV' == $_REQUEST['output_format'] && isset($_REQUEST['mssql_table']))
		{
			$mssql_table=$_REQUEST['mssql_table'];
			header('Content-type: text/comma-separated-values');
			header('Content-Disposition: attachment; filename="'.$_REQUEST['mssql_host']."_".$_REQUEST['mssql_database']."_".$mssql_table."_".date('YmdHis').'.csv"');
			_mssqldump_csv($db,$mssql_table);
			exit;
		}
	}

}

function _mssqldump_csv($db,$table)
{
	$delimiter= ",";
	if( isset($_REQUEST['csv_delimiter']))
		$delimiter= $_REQUEST['csv_delimiter'];

	if( 'Tab' == $delimiter)
		$delimiter="\t";

	if($result=sqlsrv_query($db,"select * from [$table];"))
	{
		$meta=sqlsrv_field_metadata($result);
		echo implode($delimiter,array_map(function($d){
			return $d['Name'];
		},$meta));
		echo "\n";
		while($row=sqlsrv_fetch_array($result)) {
			echo implode($delimiter,$row)."\n";
		}
	}
	sqlsrv_free_stmt($result);
}


function _mssqldump($db)
{
	if($result=sqlsrv_query($db,'select table_name from information_schema.tables where table_name not in (select table_name from  information_schema.views)'))
	{
		while($row=sqlsrv_fetch_array($result,SQLSRV_FETCH_NUMERIC))
		{
			echo "/* table $row[0] */\n";
			_mssqldump_table_data($db,$row[0]);
		}
	}
	else
	{
		echo "/* no tables in $mssql_database */\n";
	}
	sqlsrv_free_stmt($result);
}

function _mssqldump_table_data($db,$table)
{
	if(!$result=sqlsrv_query($db,"select * from [$table]")) {
		die(print_r(sqlsrv_errors(),true));
	}
	$num_rows= sqlsrv_num_rows($result);
	$num_fields= sqlsrv_num_fields($result);

	echo "/* dumping data for table `$table` */\n";
	$meta=sqlsrv_field_metadata($result);
	print_r($meta,true);

	$index=0;
	while( $row=sqlsrv_fetch_array($result,SQLSRV_FETCH_NUMERIC))
	{
		echo ($index==0)?"insert into `$table` values\n(":",\n(";
		for( $i=0; $i < $num_fields; $i++)
		{
			if( is_null( $row[$i]))
				echo "null";
			else if (is_numeric($row[$i])) {
				echo $row[$i];
			}
			else { // String, blob, other
				echo "'".str_replace("'","''",$row[$i])."'";
			}
			if( $i < $num_fields-1)
				echo ",";
		}
		echo ")";

		$index++;
	}
	echo ";\n";

	sqlsrv_free_stmt($result);
	echo "\n";
}

?><!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
<title>mssqldump.php version <?php echo $mssqldump_version; ?></title>
</head>

<body>
<?php
	foreach ($output_messages as $message)
	{
    	echo $message."<br />";
	}
?>
<form action="" method="post">
mssql connection parameters:
<table border="0">
  <tr>
    <td>Host:</td>
    <td><input  name="mssql_host" value="<?php if(isset($_REQUEST['mssql_host']))echo $_REQUEST['mssql_host']; else echo 'localhost';?>"  /></td>
  </tr>
  <tr>
    <td>Database:</td>
    <td><input  name="mssql_database" value="<?php echo $_REQUEST['mssql_database']; ?>"  /></td>
  </tr>
  <tr>
    <td>Username:</td>
    <td><input  name="mssql_username" value="<?php echo $_REQUEST['mssql_username']; ?>"  /></td>
  </tr>
  <tr>
    <td>Password:</td>
    <td><input  type="password" name="mssql_password" value="<?php echo $_REQUEST['mssql_password']; ?>"  /></td>
  </tr>
  <tr>
    <td>Output format: </td>
    <td>
      <select name="output_format" >
        <option value="SQL" <?php if( isset($_REQUEST['output_format']) && 'SQL' == $_REQUEST['output_format']) echo "selected";?> >SQL</option>
        <option value="CSV" <?php if( isset($_REQUEST['output_format']) && 'CSV' == $_REQUEST['output_format']) echo "selected";?> >CSV</option>

        </select>
    </td>
  </tr>
</table>
<button name="action" type="submit" value="mssql_test">Test Connection</button>

  <br>Dump options(SQL):

  <br>Dump options(CSV):
  <table border="0">
  <tr>
    <td>Delimiter:</td>
    <td><select name="csv_delimiter">
      <option value="," <?php if( isset($_REQUEST['output_format']) && ',' == $_REQUEST['output_format']) echo "selected";?>>,</option>
      <option value="Tab" <?php if( isset($_REQUEST['output_format']) && 'Tab' == $_REQUEST['output_format']) echo "selected";?>>Tab</option>
      <option value="|" <?php if( isset($_REQUEST['output_format']) && '|' == $_REQUEST['output_format']) echo "selected";?>>|</option>
    </select>
    </td>
  </tr>
  <tr>
    <td>Table:</td>
    <td><input  type="input" name="mssql_table" value="<?php echo $_REQUEST['mssql_table']; ?>"  /></td>
  </tr>
	<tr>
      <td>Header: </td>
      <td><input type="checkbox" name="csv_header"  <?php if(isset($_REQUEST['action']) && ! isset($_REQUEST['csv_header'])) ; else echo 'checked' ?>/></td>
    </tr>
</table>


<button name="action" type="submit" value="mssql_export">Export</button>
</form>
</body>
</html>