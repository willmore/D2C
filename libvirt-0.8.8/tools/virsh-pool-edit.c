/* Automatically generated from: virsh.c Makefile.am */
static int
cmdPoolEdit (vshControl *ctl, const vshCmd *cmd)
{
    int ret = FALSE;
    virStoragePoolPtr pool = NULL;
    char *tmp = NULL;
    char *doc = NULL;
    char *doc_edited = NULL;
    char *doc_reread = NULL;
    int flags = 0;

    if (!vshConnectionUsability(ctl, ctl->conn))
        goto cleanup;

    pool = vshCommandOptPool (ctl, cmd, "pool", NULL);
    if (pool == NULL)
        goto cleanup;

    /* Get the XML configuration of the pool. */
    doc = virStoragePoolGetXMLDesc (pool, flags);
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
        vshPrint (ctl, _("Pool %s XML configuration not changed.\n"),
                  virStoragePoolGetName (pool));
        ret = TRUE;
        goto cleanup;
    }

    /* Now re-read the pool XML.  Did someone else change it while
     * it was being edited?  This also catches problems such as us
     * losing a connection or the pool going away.
     */
    doc_reread = virStoragePoolGetXMLDesc (pool, flags);
    if (!doc_reread)
        goto cleanup;

    if (STRNEQ (doc, doc_reread)) {
        vshError(ctl,
                 "%s", _("ERROR: the XML configuration was changed by another user"));
        goto cleanup;
    }

    /* Everything checks out, so redefine the pool. */
    virStoragePoolFree (pool);
    pool = virStoragePoolDefineXML (ctl->conn, doc_edited, 0);
    if (!pool)
        goto cleanup;

    vshPrint (ctl, _("Pool %s XML configuration edited.\n"),
              virStoragePoolGetName(pool));

    ret = TRUE;

 cleanup:
    if (pool)
        virStoragePoolFree (pool);

    VIR_FREE(doc);
    VIR_FREE(doc_edited);
    VIR_FREE(doc_reread);

    if (tmp) {
        unlink (tmp);
        VIR_FREE(tmp);
    }

    return ret;
}
