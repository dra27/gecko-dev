{include file="inc/header.tpl"}

<hr class="hide">

{if $systemMessage}
<div class="key-point">
<h2>{$systemMesage.header}</h2>
<p>{$systemMessage.text|nl2br}</p>
</div>
{/if}

<div id="mBody">

{if $content}
{include file=$content}
{else}
<p>Error loading content.</p>
{if $error}
<div id="error">{$error}</div>
{/if}
<p><a href="mailto:umo-admins@mozilla.org?Subject=Broken page at {$smarty.server.PHP_SELF}">Report this problem</a></p>
{/if}

<hr class="hide">

</div>
<!-- close mBody -->

</div>
<!-- close container -->

{include file="inc/footer.tpl"}

</body>
</html>
