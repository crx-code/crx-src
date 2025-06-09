<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		xmlns:crx="http://crx.net/xsl"
		xsl:extension-element-prefixes="crx"
                version='1.0'>
<xsl:template match="/">
<xsl:value-of select="crx:functionString('foobar', /doc/@id, 'secondArg')"/>
<xsl:text>
</xsl:text>
<xsl:value-of select="crx:function('foobar', /doc/@id)"/>
<xsl:text>
</xsl:text>
<xsl:value-of select="crx:function('nodeSet')"/>
<xsl:text>
</xsl:text>
<xsl:value-of select="crx:function('nodeSet',/doc)/i"/>
<xsl:text>
</xsl:text>
<xsl:value-of select="crx:function('aClass::aStaticFunction','static')"/>
<xsl:text>
</xsl:text>

<xsl:value-of select="crx:function('nonDomNode')"/>
</xsl:template>
</xsl:stylesheet>
