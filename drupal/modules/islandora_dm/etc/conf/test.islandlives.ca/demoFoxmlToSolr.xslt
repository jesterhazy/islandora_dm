<?xml version="1.0" encoding="UTF-8"?> 
<!-- $Id: demoFoxmlToLucene.xslt 5734 2006-11-28 11:20:15Z gertsp $ -->
<xsl:stylesheet version="1.0"
		xmlns:xsl="http://www.w3.org/1999/XSL/Transform"   
    	xmlns:exts="xalan://dk.defxws.fedoragsearch.server.GenericOperationsImpl"
              xmlns:islandora-exts="xalan://ca.upei.roblib.DataStreamForXSLT"
    		exclude-result-prefixes="exts"
		xmlns:zs="http://www.loc.gov/zing/srw/"
		xmlns:foxml="info:fedora/fedora-system:def/foxml#"
		xmlns:dc="http://purl.org/dc/elements/1.1/"
		xmlns:oai_dc="http://www.openarchives.org/OAI/2.0/oai_dc/"
		xmlns:tei="http://www.tei-c.org/ns/1.0"
		xmlns:mods="http://www.loc.gov/mods/v3"
		xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" 
		xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#"
		xmlns:fedora="info:fedora/fedora-system:def/relations-external#"
		xmlns:rel="info:fedora/fedora-system:def/relations-external#" 
		xmlns:fractions="http://vre.upei.ca/fractions/"
		xmlns:compounds="http://vre.upei.ca/compounds/"
		xmlns:critters="http://vre.upei.ca/critters/"
		xmlns:clsdb="http://www.chbmr.ca/clsdb/"
		xmlns:cbmrn="http://vre.upei.ca/cbmrn/"
		xmlns:sample="http://vre.upei.ca/sample/"
		xmlns:bacteriology="http://vre.upei.ca/bacteriology/"
		xmlns:barcodedhi="http://vre.upei.ca/barcodedhi/"
		xmlns:bulktank="http://vre.upei.ca/bulktank/"
		xmlns:fc_case="http://vre.upei.ca/fc_case/"
		xmlns:mastitis_case="http://vre.upei.ca/mastitis_case/"
		xmlns:freezer="http://vre.upei.ca/freezer/"
		xmlns="http://sashimi.sourceforge.net/schema_revision/mzXML_3.1"
		xmlns:fedora-model="info:fedora/fedora-system:def/model#"
		xmlns:uvalibdesc="http://dl.lib.virginia.edu/bin/dtd/descmeta/descmeta.dtd"
		xmlns:uvalibadmin="http://dl.lib.virginia.edu/bin/admin/admin.dtd/">
	<xsl:output method="xml" indent="yes" encoding="UTF-8"/>

<!--
	 This xslt stylesheet generates the Solr doc element consisting of field elements
     from a FOXML record. The PID field is mandatory.
     Options for tailoring:
       - generation of fields from other XML metadata streams than DC
       - generation of fields from other datastream types than XML
         - from datastream by ID, text fetched, if mimetype can be handled
             currently the mimetypes text/plain, text/xml, text/html, application/pdf can be handled.
-->

	<xsl:param name="REPOSITORYNAME" select="repositoryName"/>
	<xsl:param name="FEDORASOAP" select="repositoryName"/>
	<xsl:param name="FEDORAUSER" select="repositoryName"/>
	<xsl:param name="FEDORAPASS" select="repositoryName"/>
	<xsl:param name="TRUSTSTOREPATH" select="repositoryName"/>
	<xsl:param name="TRUSTSTOREPASS" select="repositoryName"/>
	<xsl:variable name="PID" select="/foxml:digitalObject/@PID"/>
	<xsl:variable name="docBoost" select="1.4*2.5"/> <!-- or any other calculation, default boost is 1.0 -->
	
	<xsl:template match="/">
		<add> 
		<doc> 
			<xsl:attribute name="boost">
				<xsl:value-of select="$docBoost"/>
			</xsl:attribute>
		<!-- The following allows only active demo FedoraObjects to be indexed. -->
		<xsl:if test="foxml:digitalObject/foxml:objectProperties/foxml:property[@NAME='info:fedora/fedora-system:def/model#state' and @VALUE='Active']">
			<xsl:if test="not(foxml:digitalObject/foxml:datastream[@ID='METHODMAP'] or foxml:digitalObject/foxml:datastream[@ID='DS-COMPOSITE-MODEL'])">
				<xsl:if test="starts-with($PID,'')">
					<xsl:apply-templates mode="activeDemoFedoraObject"/>
				</xsl:if>
			</xsl:if>
		</xsl:if>
		</doc>
		</add>
	</xsl:template>

	<xsl:template match="/foxml:digitalObject" mode="activeDemoFedoraObject">
			<field name="PID" boost="2.5">
				<xsl:value-of select="$PID"/>
			</field>
			<xsl:for-each select="foxml:objectProperties/foxml:property">
				<field >
					<xsl:attribute name="name"> 
						<xsl:value-of select="concat('fgs.', substring-after(@NAME,'#'))"/>
					</xsl:attribute>
					<xsl:value-of select="@VALUE"/>
				</field>
			</xsl:for-each>
			<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/oai_dc:dc/*">
				<field >
					<xsl:attribute name="name">
						<xsl:value-of select="concat('dc.', substring-after(name(),':'))"/>
					</xsl:attribute>
					<xsl:value-of select="text()"/>
				</field>
			</xsl:for-each>	
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/reference/*">
				<field>
					<xsl:attribute name="name">
						<xsl:value-of select="concat('refworks.', name())"/>
					</xsl:attribute>
					<xsl:value-of select="text()"/>
				</field>
			</xsl:for-each>
		
		
		<xsl:for-each select="foxml:datastream[@ID='RIGHTSMETADATA']/foxml:datastreamVersion[last()]/foxml:xmlContent//access/human/person">
			<field >
				<xsl:attribute name="name">access.person</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>			
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream[@ID='RIGHTSMETADATA']/foxml:datastreamVersion[last()]/foxml:xmlContent//access/human/group">
			<field>
				<xsl:attribute name="name">access.group</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>			
		</xsl:for-each>
		
		<xsl:for-each select="foxml:datastream[@ID='TAGS']/foxml:datastreamVersion[last()]/foxml:xmlContent//tag">
		<!--<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent//tag">-->
			<field>
				<xsl:attribute name="name">tag</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
			<field>
				<xsl:attribute name="name">tagUser</xsl:attribute>
				<xsl:value-of select="@creator"/>
			</field>
		</xsl:for-each>
		
			<xsl:for-each select="foxml:datastream[@ID='RELS-EXT']/foxml:datastreamVersion[last()]/foxml:xmlContent//rdf:description/*">
				<field>
					<xsl:attribute name="name">
						<xsl:value-of select="concat('rels.', substring-after(name(),':'))"/>
					</xsl:attribute>
					<xsl:value-of select="@rdf:resource"/>
				</field>
			</xsl:for-each>


<!--*************************************************************full text************************************************************************************-->
		<!--<xsl:for-each select="foxml:datastream[@ID='OCR']">
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('dsm.', @ID)"/>
				</xsl:attribute>
				<xsl:value-of select="exts:getDatastreamText($PID, $REPOSITORYNAME, @ID, $FEDORASOAP, $FEDORAUSER, $FEDORAPASS, $TRUSTSTOREPATH, $TRUSTSTOREPASS)"/>
			</field>
		</xsl:for-each>-->
		

<!--***********************************************************end full text********************************************************************************-->

<!--***************************************ILIVES*****************************************************************************************************************-->

<xsl:for-each select="foxml:datastream[@ID='TEI']/foxml:datastreamVersion[last()]/foxml:xmlContent//tei:text/tei:body//*">
	<xsl:variable name="empty_string"/>
	<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('tei.', 'fullText')"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
	</xsl:if>
			
		</xsl:for-each>

<!--
		<xsl:for-each select="foxml:datastream[@ID='TEI']/foxml:datastreamVersion[last()]/foxml:xmlContent//tei:surname">
			<field>
				<xsl:attribute name="IFname">
					<xsl:value-of select="concat('tei.', 'surname')"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
				</field>

		

			
			</xsl:for-each>-->
		
		<!--<xsl:for-each select="foxml:datastream[@ID='TEI']/foxml:datastreamVersion[last()]/foxml:xmlContent//tei:forename">
			<field>
				<xsl:attribute name="IFname">
					<xsl:value-of select="concat('tei.', 'forename')"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
		

			
			</xsl:for-each>-->
		<!--<xsl:for-each select="foxml:datastream[@ID='TEI']/foxml:datastreamVersion[last()]/foxml:xmlContent//tei:settlement">
			<field>
				<xsl:attribute name="IFname">
					<xsl:value-of select="concat('tei.', 'settlement')"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
			

</xsl:for-each>-->
		<xsl:for-each select="foxml:datastream[@ID='TEI']/foxml:datastreamVersion[last()]/foxml:xmlContent//tei:date">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('tei.', 'date')"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
			</xsl:if>
			

		</xsl:for-each>
		<!--<xsl:for-each select="foxml:datastream[@ID='TEI']/foxml:datastreamVersion[last()]/foxml:xmlContent//tei:region">
			<field>
				<xsl:attribute name="IFname">
					<xsl:value-of select="concat('tei.', 'region')"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
			

</xsl:for-each>-->
		<!--<xsl:for-each select="foxml:datastream[@ID='TEI']/foxml:datastreamVersion[last()]/foxml:xmlContent//tei:addName">
			<field>
				<xsl:attribute name="IFname">
					<xsl:value-of select="concat('tei.', 'addName')"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
			

</xsl:for-each>-->
<xsl:for-each select="foxml:datastream[@ID='TEI']/foxml:datastreamVersion[last()]/foxml:xmlContent//tei:orgName">
	<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>				
				<xsl:attribute name="name">
					<xsl:value-of select="concat('tei.', 'orgName')"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
		</xsl:if>
			

		</xsl:for-each>
<xsl:for-each select="foxml:datastream[@ID='TEI']/foxml:datastreamVersion[last()]/foxml:xmlContent//tei:placeName">
	<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('tei.', 'placeName')"/>
				</xsl:attribute>
				<xsl:value-of select="tei:region"/><xsl:text> </xsl:text><xsl:value-of select="tei:settlement"/>
			</field>
			</xsl:if>

		</xsl:for-each>

<xsl:for-each select="foxml:datastream[@ID='TEI']/foxml:datastreamVersion[last()]/foxml:xmlContent//tei:persName">
	<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('tei.', 'persName')"/>
				</xsl:attribute>
				<!--<xsl:value-of select="tei:addName"/><xsl:text> </xsl:text><xsl:value-of select="tei:forename"/><xsl:text> </xsl:text><xsl:value-of select="tei:surname"/>-->
				<xsl:value-of select="tei:surname"/><xsl:text> </xsl:text><xsl:value-of select="tei:forename"/><xsl:text> </xsl:text> <xsl:value-of select="tei:addName"/>

			</field>
		</xsl:if>
			


</xsl:for-each>











<!--*************************************END ILIVES*****************************************************************************************************************-->

<!--****************************************MNPL*********************************************************************************************************************-->
		
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/fractions:sample/*">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('fraction.', substring-after(name(),':'))"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/compounds:sample/*">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field  >
				<xsl:attribute name="name">
					<xsl:value-of select="concat('compound.', substring-after(name(),':'))"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/critters:sample/*">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field  >
				<xsl:attribute name="name">
					<xsl:value-of select="concat('specimen.', substring-after(name(),':'))"/>
				</xsl:attribute>
				<xsl:value-of select="@name"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/critters:sample/critters:date_collected">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field  >
				<xsl:attribute name="name">
					<xsl:value-of select="concat('specimen.', substring-after(name(),':'))"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/critters:sample/critters:samplesize">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field  >
				<xsl:attribute name="name">
					<xsl:value-of select="concat('specimen.', substring-after(name(),':'))"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/critters:sample/critters:type">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field  >
				<xsl:attribute name="name">
					<xsl:value-of select="concat('specimen.', substring-after(name(),':'))"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/critters:sample/critters:lab_id">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field  >
				<xsl:attribute name="name">
					<xsl:value-of select="concat('specimen.', substring-after(name(),':'))"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/critters:sample/critters:description">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field  >
				<xsl:attribute name="name">
					<xsl:value-of select="concat('specimen.', substring-after(name(),':'))"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/critters:sample/critters:taxonomy/critters:phylum">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field  >
				<xsl:attribute name="name">
					<xsl:value-of select="concat('specimen.', substring-after(name(),':'))"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/critters:sample/critters:taxonomy/critters:subPhylum">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field  >
				<xsl:attribute name="name">
					<xsl:value-of select="concat('specimen.', substring-after(name(),':'))"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/critters:sample/critters:taxonomy/critters:class">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field  >
				<xsl:attribute name="name">
					<xsl:value-of select="concat('specimen.', substring-after(name(),':'))"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/critters:sample/critters:taxonomy/critters:order">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field  >
				<xsl:attribute name="name">
					<xsl:value-of select="concat('specimen.', substring-after(name(),':'))"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/critters:sample/critters:taxonomy/critters:family">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field  >
				<xsl:attribute name="name">
					<xsl:value-of select="concat('specimen.', substring-after(name(),':'))"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/critters:sample/critters:taxonomy/critters:genus">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field  >
				<xsl:attribute name="name">
					<xsl:value-of select="concat('specimen.', substring-after(name(),':'))"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/critters:sample/critters:taxonomy/critters:species">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field  >
				<xsl:attribute name="name">
					<xsl:value-of select="concat('specimen.', substring-after(name(),':'))"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/critters:sample/critters:site/critters:sitename">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field  >
				<xsl:attribute name="name">
					<xsl:value-of select="concat('specimen.', substring-after(name(),':'))"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/critters:sample/critters:site/critters:country">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field  >
				<xsl:attribute name="name">
					<xsl:value-of select="concat('specimen.', substring-after(name(),':'))"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/critters:sample/critters:site/critters:region">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field  >
				<xsl:attribute name="name">
					<xsl:value-of select="concat('specimen.', substring-after(name(),':'))"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/critters:sample/critters:site/critters:latitude">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field  >
				<xsl:attribute name="name">
					<xsl:value-of select="concat('specimen.', substring-after(name(),':'))"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/critters:sample/critters:site/critters:longitude">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field  >
				<xsl:attribute name="name">
					<xsl:value-of select="concat('specimen.', substring-after(name(),':'))"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/critters:sample/critters:site/critters:depth">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field  >
				<xsl:attribute name="name">
					<xsl:value-of select="concat('specimen.', substring-after(name(),':'))"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/critters:sample/critters:collectors/critters:collector">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field  >
				<xsl:attribute name="name">
					<xsl:value-of select="concat('specimen.', substring-after(name(),':'))"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		
		
		
		
<!--**************************************END MNPL************************************************************************************************************************-->

<!--****************************************CHBMR*********************************************************************************************************************-->
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/clsdb:record/*">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
				<field >
					<xsl:attribute name="name">
						<xsl:value-of select="concat('chbmr.', substring-after(name(),':'))"/>
					</xsl:attribute>
					<xsl:value-of select="text()"/>
				</field>
			</xsl:if>
		</xsl:for-each>
		
<!--**************************************END CHBMR*******************************************************************************************************************-->

<!--****************************************CBMRN (Mastitis)**********************************************************************************************************-->

		<!-- Sample Bacteriology... as in, bacteriology of samples -->
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/bacteriology:sample/*">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
				<field >
					<xsl:attribute name="name">
						<xsl:value-of select="concat('sample.', substring-after(name(),':'))"/>
					</xsl:attribute>
					<xsl:value-of select="text()"/>
				</field>
			</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/bacteriology:sample/bacteriology:culturedate">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
				<field >
					<xsl:attribute name="name">
						<xsl:value-of select="concat('sample.', 'culturedate', '.solrdate')"/>
					</xsl:attribute>
					<xsl:value-of select="concat(text(),'T00:00:00Z')"/>
				</field>
			</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/bacteriology:sample/bacteriology:isolate/*">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
				<field >
					<xsl:attribute name="name">
						<xsl:value-of select="concat('sample.', substring-after(name(),':'))"/>
					</xsl:attribute>
					<xsl:value-of select="text()"/>
				</field>
			</xsl:if>
		</xsl:for-each>

		<!-- BarcodeDHI, mixed in with Bacteriology -->
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/barcodedhi:sample/barcodedhi:herdprid">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
				<field >
					<xsl:attribute name="name">
						<xsl:value-of select="concat('sample.', substring-after(name(),':'))"/>
					</xsl:attribute>
					<xsl:value-of select="text()"/>
				</field>
			</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/barcodedhi:sample/barcodedhi:dhinum">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
				<field >
					<xsl:attribute name="name">
						<xsl:value-of select="concat('sample.', substring-after(name(),':'))"/>
					</xsl:attribute>
					<xsl:value-of select="text()"/>
				</field>
			</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/barcodedhi:sample/barcodedhi:dhicownumber">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
				<field >
					<xsl:attribute name="name">
						<xsl:value-of select="concat('sample.', substring-after(name(),':'))"/>
					</xsl:attribute>
					<xsl:value-of select="text()"/>
				</field>
			</xsl:if>
		</xsl:for-each>

		<!-- Bulktank -->
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/bulktank:sample/*">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
				<xsl:if test="name = 'bulktank:barcode'"> <!-- don't double-index the identical barcode -->
					<field >
						<xsl:attribute name="name">
							<xsl:value-of select="concat('bulktank.', substring-after(name(),':'))"/>
						</xsl:attribute>
						<xsl:value-of select="text()"/>
					</field>
				</xsl:if>
			</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/bulktank:sample/bulktank:besc/*">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
				<field >
					<xsl:attribute name="name">
						<xsl:value-of select="concat('bulktank.best_', substring-after(name(),':'))"/>
					</xsl:attribute>
					<xsl:value-of select="text()"/>
				</field>
			</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/bulktank:sample/bulktank:dvm/*">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
				<field >
					<xsl:attribute name="name">
						<xsl:value-of select="concat('bulktank.dvm_', substring-after(name(),':'))"/>
					</xsl:attribute>
					<xsl:value-of select="text()"/>
				</field>
			</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/bulktank:sample/bulktank:mac/*">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
				<field >
					<xsl:attribute name="name">
						<xsl:value-of select="concat('bulktank.mac_', substring-after(name(),':'))"/>
					</xsl:attribute>
					<xsl:value-of select="text()"/>
				</field>
			</xsl:if>
		</xsl:for-each>
		
		<!-- Freezer Locations -->
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/freezer:locations/freezer:location">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
				<field >
					<xsl:attribute name="name">
						<xsl:value-of select="concat('freezer.', 'box')"/>
					</xsl:attribute>
					<xsl:value-of select="@box"/>
				</field>
				<field >
					<xsl:attribute name="name">
						<xsl:value-of select="concat('freezer.', 'grid')"/>
					</xsl:attribute>
					<xsl:value-of select="@grid"/>
				</field>
			</xsl:if>
		</xsl:for-each>
		
		<!-- FC Cases -->
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/fc_case:case/*">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
				<field >
					<xsl:attribute name="name">
						<xsl:value-of select="concat('fc_case.', substring-after(name(),':'))"/>
					</xsl:attribute>
					<xsl:value-of select="text()"/>
				</field>
			</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/fc_case:case/fc_case:sample/fc_case:date">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
				<field >
					<xsl:attribute name="name">
						<xsl:value-of select="concat('fc_case.', 'sample_date')"/>
					</xsl:attribute>
					<xsl:value-of select="text()"/>
				</field>
			</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/fc_case:case/fc_case:sample/fc_case:barcode">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
				<field >
					<xsl:attribute name="name">
						<xsl:value-of select="concat('fc_case.', 'sample_barcode')"/>
					</xsl:attribute>
					<xsl:value-of select="text()"/>
				</field>
			</xsl:if>
		</xsl:for-each>
		
		<!-- Mastitis Cases -->
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/mastitis_case:case/*">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
				<field >
					<xsl:attribute name="name">
						<xsl:value-of select="concat('mastitis_case.', substring-after(name(),':'))"/>
					</xsl:attribute>
					<xsl:value-of select="text()"/>
				</field>
			</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/mastitis_case:case/mastitis_case:sample/mastitis_case:barcode">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
				<field >
					<xsl:attribute name="name">
						<xsl:value-of select="concat('mastitis_case.', 'sample_barcode')"/>
					</xsl:attribute>
					<xsl:value-of select="text()"/>
				</field>
			</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/mastitis_case:case/mastitis_case:sample/mastitis_case:date">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
				<field >
					<xsl:attribute name="name">
						<xsl:value-of select="concat('mastitis_case.', 'sample_date')"/>
					</xsl:attribute>
					<xsl:value-of select="text()"/>
				</field>
			</xsl:if>
		</xsl:for-each>
		
<!--**************************************END CBMRN (Mastitis)********************************************************************************************************-->

<!--**************************************START Islandora-DM**********************************************************************************************************-->

		<xsl:if test="starts-with($PID,'islandora-dm')">

			<!-- rels-ext handling added here, because the stuff around line 128 doesn't seem to work -->
			<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/rdf:RDF/rdf:Description/*">
				<field>
					<xsl:attribute name="name">
						<xsl:value-of select="concat('rels.', name())"/>
					</xsl:attribute>
					<xsl:value-of select="substring-after(@rdf:resource, '/')"/>
				</field>
			</xsl:for-each>
	
			<!--  which dm-specific dsids are indexed (across all content models)? -->
			<xsl:variable name="indexed-dsids" select="'/dm/uis/note/xml/text/'" />
	
			<!-- indexing for inline xml streams -->
			<xsl:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent/*">
				<xsl:variable name="dsid" select="../../../@ID" />
				<xsl:if test="contains($indexed-dsids, concat('/', $dsid, '/'))">
					<field>
						<xsl:attribute name="name">islandora-dm.po</xsl:attribute>
						<xsl:value-of select="normalize-space(.)"/>
					</field>

					<!-- special handling for specific streams -->
					<xsl:if test="$dsid = 'dm'">
						<field>
							<xsl:attribute name="name">islandora-dm.po.owner</xsl:attribute>
							<xsl:value-of select="normalize-space(owner)"/>
						</field>
					</xsl:if>
			
					<xsl:if test="$dsid = 'uis'">
						<field>
							<xsl:attribute name="name">islandora-dm.po.status</xsl:attribute>
							<xsl:value-of select="normalize-space(processed)"/>
						</field>
					</xsl:if>
				</xsl:if>
			</xsl:for-each>
			
			<!-- indexing for external text streams (plain or xml) -->
			<xsl:for-each select="foxml:datastream[@CONTROL_GROUP = 'M']">
				<xsl:variable name="dsid" select="@ID" />
				<xsl:variable name="mime" select="./foxml:datastreamVersion[last()]/@MIMETYPE" />
				
				<xsl:if test="contains($indexed-dsids, concat('/', $dsid, '/'))">
				
					<xsl:variable name="content">
						<xsl:if test="$mime = 'text/plain'">
							<xsl:value-of select="islandora-exts:getDatastreamTextRaw($PID, $REPOSITORYNAME, $dsid, $FEDORASOAP, $FEDORAUSER, $FEDORAPASS, $TRUSTSTOREPATH, $TRUSTSTOREPASS)" />
						</xsl:if>
						<xsl:if test="$mime = 'text/xml'">
							<xsl:value-of select="islandora-exts:getXMLDatastreamASNodeList($PID, $REPOSITORYNAME, $dsid, $FEDORASOAP, $FEDORAUSER, $FEDORAPASS, $TRUSTSTOREPATH, $TRUSTSTOREPASS)" />
						</xsl:if>
					</xsl:variable>
				
					<field>
						<xsl:attribute name="name">islandora-dm.po</xsl:attribute>
						<xsl:value-of select="normalize-space($content)"/>
					</field>
				</xsl:if>
			</xsl:for-each>
		</xsl:if>

<!--**************************************END Islandora-DM************************************************************************************************************-->


<!--*********************************** begin changes for Mods as a managed datastream users an islandor extension function********************************************************************************-->
<xsl:for-each select="foxml:datastream[@ID='MODS']/foxml:datastreamVersion[last()]">
			<xsl:call-template name="mods"/> <!--only call this if the mods stream exists-->			
</xsl:for-each>	
</xsl:template>
<xsl:template name="mods">
<xsl:variable name="MODS_STREAM" select="islandora-exts:getXMLDatastreamASNodeList($PID, $REPOSITORYNAME, 'MODS', $FEDORASOAP, $FEDORAUSER, $FEDORAPASS, $TRUSTSTOREPATH, $TRUSTSTOREPASS)"/>
<!--***********************************************************MODS modified for maps**********************************************************************************-->
<xsl:for-each select="$MODS_STREAM//mods:title">
	<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('mods.', 'title')"/>
				</xsl:attribute>
				<xsl:value-of select="../mods:nonSort/text()"/><xsl:text> </xsl:text><xsl:value-of select="text()"/>
			</field>
			</xsl:if>

		</xsl:for-each>
<xsl:for-each select="$MODS_STREAM//mods:subTitle">
	<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('mods.', 'subTitle')"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
		</xsl:if>
			

</xsl:for-each>
		<xsl:for-each select="$MODS_STREAM//mods:abstract">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('mods.', name())"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
			
			
		</xsl:for-each>
		
		<xsl:for-each select="$MODS_STREAM//mods:genre">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
				<field>
					<xsl:attribute name="name">
						<xsl:value-of select="concat('mods.', name())"/>
					</xsl:attribute>
					<xsl:value-of select="text()"/>
				</field>
			</xsl:if>
			
			
		</xsl:for-each>
		<xsl:for-each select="$MODS_STREAM//mods:form">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('mods.', name())"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
			
			
		</xsl:for-each>
<xsl:for-each select="$MODS_STREAM//mods:roleTerm">
	<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('mods.', text())"/>					
				</xsl:attribute>
				<xsl:value-of select="../../mods:namePart/text()"/>
			</field>
			</xsl:if>

		</xsl:for-each>

<xsl:for-each select="$MODS_STREAM//mods:note[@type='statement of responsibility']">
	<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('mods.', 'sor')"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
			</xsl:if>

</xsl:for-each>
		<xsl:for-each select="$MODS_STREAM//mods:note">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('mods.', 'note')"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
			</xsl:if>
			
		</xsl:for-each>

<xsl:for-each select="$MODS_STREAM//mods:subject/*">
	<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('mods.', 'subject')"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
		</xsl:if>
	</xsl:for-each>
	<xsl:for-each select="$MODS_STREAM//mods:country">
		<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
		<field>
			<xsl:attribute name="name">
				<xsl:value-of select="concat('mods.', 'country')"/>
			</xsl:attribute>
			<xsl:value-of select="text()"/>
		</field>
			</xsl:if>
	</xsl:for-each>
		<xsl:for-each select="$MODS_STREAM//mods:province">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('mods.', 'province')"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="$MODS_STREAM//mods:county">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('mods.', 'county')"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="$MODS_STREAM//mods:region">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('mods.', 'county')"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="$MODS_STREAM//mods:city">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('mods.', 'county')"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="$MODS_STREAM//mods:citySection">
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('mods.', 'county')"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
				</xsl:if>
		</xsl:for-each>
		<xsl:for-each select="$MODS_STREAM//mods:subject/mods:name/mods:namePart/*">	
			<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('mods.', 'subject')"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
			</xsl:if>

		</xsl:for-each>


<xsl:for-each select="$MODS_STREAM//mods:physicalDescription/*">
	<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('mods.', name())"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
			</xsl:if>

		</xsl:for-each>		

<xsl:for-each select="$MODS_STREAM//mods:originInfo//mods:placeTerm[@type='text']">
	<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('mods.', 'place_of_publication')"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
		</xsl:if>	
		</xsl:for-each>	

<xsl:for-each select="$MODS_STREAM//mods:originInfo/mods:publisher">
	<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('mods.', name())"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
		</xsl:if>	

</xsl:for-each>	
<xsl:for-each select="$MODS_STREAM//mods:originInfo/mods:edition">
	<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('mods.', name())"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
		</xsl:if>	

		</xsl:for-each>		

<xsl:for-each select="$MODS_STREAM//mods:originInfo/mods:dateIssued">
	<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('mods.', name())"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
			</xsl:if>

		</xsl:for-each>	
		
<xsl:for-each select="$MODS_STREAM//mods:originInfo/mods:issuance">
	<xsl:if test="text() [normalize-space(.) ]"><!--don't bother with empty space-->
			<field>
				<xsl:attribute name="name">
					<xsl:value-of select="concat('mods.', name())"/>
				</xsl:attribute>
				<xsl:value-of select="text()"/>
			</field>
		</xsl:if>	

</xsl:for-each>	</xsl:template>
</xsl:stylesheet>	
