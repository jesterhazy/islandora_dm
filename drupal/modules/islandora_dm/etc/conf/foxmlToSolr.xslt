<!-- 
  xslt used by gsearch to convert foxml to solr updates.
  fragments meant to be inserted into a file like demoFoxmlToSolr.xslt
  
  todo:
    - index xacml so results can be limited by access rules
    - index po record and documents differently
    - capture ocr text etc. from scanned docs
    - skip po-collection object
-->

<!-- add this to "/" template, as sibling of <xsl:if test="starts-with($PID,'demo')">... --> 
<xsl:if test="starts-with($PID,'islandora-dm')">
  <xsl:apply-templates mode="islandora-dm"/>
</xsl:if>


<!-- 
  add this as sibling of "/" template 
-->
<xsl:template match="/foxml:digitalObject" mode="islandora-dm">
    <field name="PID" boost="2.5">
      <xsl:value-of select="$PID"/>
    </field>

    <xsl:for-each select="foxml:objectProperties/foxml:property">
      <field>
        <xsl:attribute name="name"> 
          <xsl:value-of select="concat('fgs.', substring-after(@NAME,'#'))"/>
        </xsl:attribute>
        <xsl:value-of select="@VALUE"/>
      </field>
    </xsl:for-each>

    <xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/oai_dc:dc/*">
      <field>
        <xsl:attribute name="name">
          <xsl:value-of select="concat('dc.', substring-after(name(),':'))"/>
        </xsl:attribute>
        <xsl:value-of select="text()"/>
      </field>
    </xsl:for-each>


    <xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/*:RDF/*:Description/*">
      <xsl:if test="name() = 'hasModel'">
        <field>
          <xsl:attribute name="name">islandora-dm.po.cmodel</xsl:attribute>
          <xsl:value-of select="substring-after(@*:resource, '/')"/>
        </field>
      </xsl:if>
      <xsl:if test="name() = 'isMemberOf'">
        <field>
          <xsl:attribute name="name">islandora-dm.po.collection</xsl:attribute>
          <xsl:value-of select="substring-after(@*:resource, '/')"/>
        </field>
      </xsl:if>
    </xsl:for-each>    
    
    <xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/dm/*">
      <xsl:if test="name() = 'owner'">
        <field>
          <xsl:attribute name="name">islandora-dm.po.owner</xsl:attribute>
          <xsl:value-of select="."/>
        </field>
      </xsl:if>
      <field>
        <xsl:attribute name="name">islandora-dm.po</xsl:attribute>
        <xsl:value-of select="."/>
      </field>
    </xsl:for-each>  

    <xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/po/*">
      <xsl:if test="name() = 'processed'">
        <field>
          <xsl:attribute name="name">islandora-dm.po.status</xsl:attribute>
          <xsl:value-of select="."/>
        </field>
      </xsl:if>
      <field>
        <xsl:attribute name="name">islandora-dm.po</xsl:attribute>
        <xsl:value-of select="."/>
      </field>
    </xsl:for-each>

  </xsl:template>