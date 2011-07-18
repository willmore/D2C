/* Automatically generated from: virsh.c Makefile.am */
static int
cmdNetworkEdit (vshControl *ctl, const vshCmd *cmd)
{
    int ret = FALSE;
    virNetworkPtr network = NULL;
    char *tmp = NULL;
    char *doc = NULL;
    char *doc_edited = NULL;
    char *doc_reread = NULL;
    int flags = 0;

    if (!vshConnectionUsability(ctl, ctl->conn))
        goto cleanup;

    network = vshCommandOptNetwork (ctl, cmd, NULL);
    if (network == NULL)
        goto cleanup;

    /* Get the XML configuration of the network. */
    doc = virNetworkGetXMLDesc (network, flags);
    if (!doc)
        goto cleanup;

    /* Create and open the temporary file. */
    tmp = editWriteToTempFile (ctl, doc);
    if (!tmp) goto cleanup;

    /* Start the editor. */
    if (editFile (ctl, tmp) == -1) goto cleanup;

    /* Read back the edited file. */
    doc_edited = editReadBackFile (ctl, tmp);
    if (!doc_edited) goto cleanup;

    /* Compare original XML with edited.  Has it changed at all? */
    if (STREQ (doc, doc_edited)) {
        vshPrint (ctl, _("Network %s XML configuration not changed.\n"),
                  virNetworkGetName (network));
        ret = TRUE;
        goto cleanup;
    }

    /* Now re-read the network XML.  Did someone else change it while
     * it was being edited?  This also catches problems such as us
     * losing a connection or the network going away.
     */
    doc_reread = virNetworkGetXMLDesc (network, flags);
    if (!doc_reread)
        goto cleanup;

    if (STRNEQ (doc, doc_reread)) {
        vshError(ctl,
                 "%s", _("ERROR: the XML configuration was changed by another user"));
        goto cleanup;
    }

    /* Everything checks out, so redefine the network. */
    virNetworkFree (network);
    network = virNetworkDefineXML (ctl->conn, doc_edited);
    if (!network)
        goto cleanup;

    vshPrint (ctl, _("Network %s XML configuration edited.\n"),
              virNetworkGetName(network));

    ret = TRUE;

 cleanup:
    if (network)
        virNetworkFree (network);

    VIR_FREE(doc);
    VIR_FREE(doc_edited);
    VIR_FREE(doc_reread);

    if (tmp) {
        unlink (tmp);
        VIR_FREE(tmp);
    }

    return ret;
}
