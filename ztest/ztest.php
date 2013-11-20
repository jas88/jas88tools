<?php
if (isset($_GET['op'])) {
	header('Content-Type: text/javascript');
	switch($_GET['op']) {
		case 'gzip':
		header('Content-Encoding: gzip');
		$output=<<<EOO
document.getElementById('gzipok').className='good';
document.getElementById('gzipto').className='hidden';
EOO;
		echo gzencode($output,9,FORCE_GZIP);
		break;
		
		case 'zlib':
		header('Content-Encoding: deflate');
		$output=<<<EOO
document.getElementById('zlibok').className='good';
document.getElementById('zlibto').className='hidden';
EOO;
		echo gzcompress($output,9,ZLIB_ENCODING_DEFLATE);
		break;

		case 'null':
		$output=<<<EOO
document.getElementById('nullto').className='hidden';
document.getElementById('nullok').className='good';
EOO;
		echo $output;
		break;

		default:
		echo "alert('Invalid operation requested');";
	}
	exit;
}

$jsdata=<<<EOJ
document.getElementById('datato').className='hidden';
document.getElementById('dataok').className='good';
EOJ;
$jsdata="data:text/javascript;base64,".base64_encode($jsdata);

if (!isset($_SERVER['HTTP_ACCEPT_ENCODING'])) {
	$support='NO compression systems';
} else {
	$support=$_SERVER['HTTP_ACCEPT_ENCODING'];
}
header('Content-Type: text/html; charset=utf-8');

?><!DOCTYPE html>
<html>
<head>
	<title>HTTP compression support test page</title>
	<style type="text/css">
	.hidden { display: none; }
	.bad { color: red; font-weight: bold; }
	</style>
</head>
<body>
	<h1>HTTP compression support test page</h1>
	<ul>
		<li>Your browser claimed to support: <?php echo $support; ?></li>
		<li class="bad hidden">CSS support: this line should never be visible on working browsers. If visible, this invalidates results below.</li>
		<li class="bad" id="nullto">Javascript: the tests have failed to load at all, or are still loading</li>
		<li class="bad" id="datato">This browser FAILED to load data-encoded content</li>
		<li class="good hidden" id="dataok">This browser loaded data-encoded content OK</li>
		<li class="bad" id="gzipto">This browser FAILED to load gzip-encoded content</li>
		<li class="good hidden" id="gzipok">This browser loaded gzip-encoded content OK</li>
		<li class="bad" id="zlibto">This browser FAILED to load zlib-encoded content</li>
		<li class="good hidden" id="zlibok">This browser loaded zlib-encoded content OK</li>
		<li class="good hidden" id="nullok">All tests complete.</li>
	</ul>
	<p>On most modern browsers, you should see 'gzip,deflate' offered on the first line, followed by confirmation both encodings worked properly and all tests completed.</p>
	<p>Google Chrome derivates will also offer Google's <a href="http://en.wikipedia.org/wiki/Shared_Dictionary_Compression_Over_HTTP">sdch</a> encoding. This page does not currently test that, merely notes it for information.</p>
	<p>Internet Explorer - at least as far as version 10 - will fail the zlib test, since Microsoft confused RFC 1950 and RFC 1951.</p>
	<script type="text/javascript" src="<?php echo $jsdata; ?>"></script>
	<script type="text/javascript" src="./?op=gzip"></script>
	<script type="text/javascript" src="./?op=zlib"></script>
	<script type="text/javascript" src="./?op=null"></script>
</body>
</html>