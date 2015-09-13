var Messenger = new function () {
    var latestBlock = 0;
    var messages = [];
    this.switchToContact = function (id) {
        $('#' + currentContact).removeClass("active");
        $('#' + id).addClass('active');
        currentContact = id;
        var pdiv = $("#poster-tpl").clone(true, true).removeAttr("id");
        pdiv.find(".id-follow-btn").parent().remove();
        pdiv.find(".id-unfollow-btn").parent().remove();
        pdiv.find(".id-chat-btn").parent().remove();
        pdiv.find(".id-share-btn").parent().remove();
        pdiv.find(".poster").attr("id", id);
        pdiv.find(".id").find("a.text").html(id);
        $("#msg-header").addClass("msg-ha");
        var domain = FAI_API.getDomainByForward(id);
        var id2show = $.isEmptyObject(domain) ? CUtil.getShortPId(id) : (domain.alias ? domain.alias + " (" + domain.domain + ")" : domain.domain);
        id2show = contacts[currentContact].alias ? contacts[currentContact].alias : id2show;
        var intro = $.isEmptyObject(domain) ? "" : " " + domain.intro;
        var idtype = $.isEmptyObject(domain) ? "" : "(domain)";
        pdiv.find(".id").find(".text").html(id2show);
        pdiv.find(".id").find(".text").attr("fullid", id);
        if (!$.isEmptyObject(domain))
            pdiv.find(".id").find(".text").attr("domain", domain.domain);
        pdiv.find(".id").find(".idtype").html(idtype + intro);
        if (domain.icon) {
            console.log(domain.icon);
            var iconCtt = FAI_API.getContentByLink(domain.icon);
            var iconCttP = CUtil.parseCtt(iconCtt);
            var icon = CBrowser.createImgHtml(iconCttP);
            console.log(icon);
            pdiv.find(".icon").html(icon);
        }
        $("#current-contact").html("").prepend(pdiv.children());
        console.log('edi')
        $("#btn-edit-ctct").removeClass("hide").unbind().click(function () {
            console.log('ed')
            Messenger.showModal("edit");
        });
        console.log($("#btn-edit-ctct"));
        this.showMessages(id);
    }

    this.showContactInList = function (id) {
        var a = this.readContactInfo(id);
        if (a.alias)
            contacts[id].alias = a.alias;
        if (a.icon)
            contacts[id].icon = a.icon;
        if (a.intro)
            contacts[id].intro = a.intro;
        if (a.category)
            contacts[id].category = a.category;
        var domain = FAI_API.getDomainByForward(id);
        console.log(domain);
        if (domain && domain.domain) {
            contacts[id].domainName = domain.domain;
            if (!contacts.intro && domain.intro)
                contacts[id].intro = domain.intro;
            if (!contacts.icon && domain.icon)
                contacts[id].icon = {link: domain.icon};
        }
        else {
            contacts[id].domainName = "";
        }
        var pdiv = $("#poster-tpl").clone(true, true).removeAttr("id");
        pdiv.find(".poster").attr("id", id);
        $("#contact-list").append(pdiv.children().addClass("ctct").click(function () {
            Messenger.switchToContact(id);
        }));
        this.updateContactDisplay(id);
    }
    this.updateContactDisplay = function (id) {
        if (!contacts[id])
            return;
        var c = contacts[id];
        if (c.icon) {
            var iconCtt = FAI_API.getContentByLink(c.icon.link);
            var iconCttP = CUtil.parseCtt(iconCtt);
            var icon = CBrowser.createImgHtml(iconCttP);
            contacts[id].icon.data = iconCttP.fdata;
            contacts[id].icon.type = iconCttP.ftype;
            $("#" + id).find(".icon").html(icon);
        }
        var id2show;
        if (c.alias)
            id2show = c.alias;
        else if (c.domainName)
            id2show = c.domainName;
        else
            id2show = showID(id);
        id2show += "<div class='ntcno1'></div>";
        $("#" + id).find(".id").html(id2show);
        this.updateUnreadLen(id);
    }
    this.getMsgHtml = function (msg, icon, time, mode, direction) {
        var mdiv = $("#msg-tpl").clone(true, true).removeAttr("id").removeClass("hide");
        var lr = (direction == "in") ? "left" : "right";
//        var rlr = (direction == "in") ? "right" : "left";
        mdiv.find(".icon").html(icon).css("float", lr);
        mdiv.find(".msg-wrapper").css("float", lr).addClass("bubble-" + lr).css("margin-" + lr, 30).addClass("msg-color-" + lr);
        mdiv.find(".time").html(CUtil.dateToShortString(time)).css("float", lr);
        mdiv.find(".msg").html(msg).css("float", lr).css("clear", lr);
        return mdiv;
    };
    this.getIcon = function (id) {
        var r = $("<div />");
        var img;
        if (id === gParam.accountID) {
            img = gParam.icon ? CPage.createImgHtml(gParam.icon.type, gParam.icon.data) : this.getIconFrId(id);
        } else {
            var c = contacts[id];
            if (!c) {
                img = this.getIconFrId(id);
                console.log(c);
            } else if (c.icon) {
                img = CPage.createImgHtml(c.icon.type, c.icon.data);
            }
            else
                img = this.getIconFrId(id);
        }
        r.html(img).attr("title", id);
        return r;
    };
    this.getIconFrId = function (id) {
        return $("<div />").html(id.substr(0, 3)).css("font-weight", "bold").addClass("grey_8").css("padding-left", 2);
    };
    this.showMessage = function (msg, idForeign, time, mode, direction, fOld) {
        mode = typeof mode === "undefined" ? "" : mode;
        var icon = direction === "in" ? this.getIcon(idForeign) : this.getIcon(gParam.accountID);
        if (!time)
            time = new Date();
        var html = this.getMsgHtml(msg, icon, time, mode, direction);
        if (!fOld) {
            $('#msg-history').append(html);
            $('#msg-history').scrollTop($('#msg-history').scrollTop() + $('#msg-history').innerHeight());
        } else {
            $('#msg-history').prepend(html);
        }
    };
    this.showMessages = function (id) {
        $('#msg-history').html("");
        messages[id] = FAI_API.getMessages(gParam.accountID, [id], 0, true, false, 0, 0, 100);
        if (this.decryptAndShow(id, messages[id])) {
            contacts[id].offset = messages[id].length;
            contacts[id].unreadLen = 0;
            this.setUnreadLen(id, 0);
        }
    }
    this.showOldMessages = function () {
        var id = currentContact;
        var oldMsgs = FAI_API.getMessages(gParam.accountID, [id], 0, true, false, 0, contacts[id].offset, 20);
        messages[id] = oldMsgs.concat(messages[id]);
        console.log(messages[id].length);
        this.decryptAndShow(id, oldMsgs, true);
        contacts[id].offset += oldMsgs.length;
    }

    this.decryptAndShow = function (id, msgs, fOld) {
        var msgs2 = [];
        for (var j in msgs) {
            msgs2.push(msgs[j].content);
        }
        var rs = false;
        FAI_API.decryptMessages(gParam.accountID, [{idForeign: id, messages: msgs2}], function (decryptmsgs) {
            if (fOld) {
                for (var j = 0; j < decryptmsgs[0].messages.length; j++) {
                    if (decryptmsgs[0].messages[j]) {
                        var direction = FAI_API.areIDsEqual(msgs[j].IDTo, id) ? "out" : "in";
                        if (decryptmsgs[0].messages[j][0]) {
                            msgs[j].decrypted = base64.decode(decryptmsgs[0].messages[j][0].content[0].content);
                            Messenger.updateMessage(msgs[j]);
                            var t = new Date(msgs[j].nTime * 1000);
                            Messenger.showMessage(msgs[j].decrypted, id, t, msgs[j].mode, direction, fOld);
                        }
                    }
                }
            } else {
                for (var j = decryptmsgs[0].messages.length - 1; j >= 0; j--) {
                    if (decryptmsgs[0].messages[j]) {
                        var direction = FAI_API.areIDsEqual(msgs[j].IDTo, id) ? "out" : "in";
                        if (decryptmsgs[0].messages[j][0]) {
                            msgs[j].decrypted = base64.decode(decryptmsgs[0].messages[j][0].content[0].content);
                            Messenger.updateMessage(msgs[j]);
                            var t = new Date(msgs[j].nTime * 1000);
                            Messenger.showMessage(msgs[j].decrypted, id, t, msgs[j].mode, direction, fOld);
                        }
                    }
                }
            }
            Messenger.setLastUpdateTime(id);
            rs = true;
        }, function (e) {
            console.log(e);
            rs = false;
        });
        return rs;
    };
    this.updateMessage = function (msg) {
        if (msg.IDForeign) {
            for (var j in messages[msg.IDForeign]) {
                var msg2 = messages[msg.IDForeign][j];
                if (msg2.txid == msg.txid && msg2.nVout == msg.nVout)
                    msg2.decrypted = msg.decrypted;
            }
        }
    };
    this.addContact = function (id) {
        console.log(id);
        if (contacts[id] || id == accountID)
            return false;
        contacts[id] = {};
        Messenger.showContactInList(id);
        return true;
    }
    this.addMessage = function (msg) {
        msg.IDForeign = FAI_API.areIDsEqual(msg.IDFrom, gParam.accountID) ? msg.IDTo : msg.IDFrom;
        if (this.hasMessage(msg))
            return;
        if (this.addContact(msg.IDForeign))
            this.saveContacts();
        if (!messages[msg.IDForeign]) {
            messages[msg.IDForeign] = [];
        }
        messages[msg.IDForeign].push(msg);
        this.updateUnreadLen(msg.IDForeign, 1);
    }
    this.getLastUpdateTime = function (id) {
        var t = FAI_API.getConf("messenger", gParam.accountID, id, "updatetime");
        if (!t || isNaN(t))
            return 0;
        return t;
    }
    this.setLastUpdateTime = function (id) {
        return FAI_API.setConf("messenger", gParam.accountID, id, "updatetime", String(new Date().getTime()));
    }
    this.updateUnreadLen = function (id, n) {
        if (typeof n == "undefined")
            n = 0;
        if (!contacts[id])
            return false;
        if (!contacts[id].unreadLen)
            this.getUnreadLen(id);
        contacts[id].unreadLen += n;
        var a = contacts[id].unreadLen;
        if (!a)
            a = "";
        $("#" + id).find(".ntcno1").html(a);
    }
    this.getUnreadLen = function (id) {
        var l = FAI_API.getConf("messenger", gParam.accountID, id, "unreadlen");
        if (typeof l === "undefined")
            l = 0;
        contacts[id].unreadLen = Number(l);
    }
    this.setUnreadLen = function (id, n) {
        if (!n)
            n == "0";
        contacts[id].unreadLen = n;
        this.saveContacts();
        FAI_API.setConf("messenger", gParam.accountID, id, "unreadlen", n);
        if (n == "0")
            n = "";
        $("#" + id).find(".ntcno1").html(n);
    }
    this.getAlias = function (id) {
        var alias = FAI_API.getConf("messenger", gParam.accountID, id, "alias");
        if (alias.error)
            alias = "";
        return alias;
    }
    this.setAlias = function (id, alias) {
        return FAI_API.setConf("messenger", gParam.accountID, id, "alias", alias);
    }
    this.getCategory = function (id) {
        var category = FAI_API.getConf("messenger", gParam.accountID, id, "category");
        if (category.error)
            category = "";
        return category;
    }
    this.setCategory = function (id, category) {
        return FAI_API.setConf("messenger", gParam.accountID, id, "category", category);
    }
    this.readContactInfo = function (id) {
        var c = {};
        c.alias = this.getAlias(id);
        c.updateTime = this.getLastUpdateTime(id);
        c.category = this.getCategory(id);
        return c;
    }
    this.getMessagesAll = function () {
        lastBlock = FAI_API.getConf("messenger", gParam.accountID, "", "lastUpdateBlock");
        if (typeof lastBlock === "undefined")
            lastBlock = 0;
        contacts = $.parseJSON(FAI_API.readFile("messenger", gParam.accountID, "contacts"));
        if (!contacts || typeof contacts === "undefined" || contacts.length == 0)
            contacts = {};
        console.log(contacts);
        for (var id in contacts)
            this.showContactInList(id);
        var data = FAI_API.getMessages(gParam.accountID, null, 0, true, true, Number(lastBlock) + 1, 0, 100000);

        console.log(data);
        if (data && !data.error) {
            for (var l in data) {
                this.addContact(l);
                this.updateUnreadLen(l, data[l]);
            }
            this.saveContacts();
        }
        latestBlock = FAI_API.getBlockCount();
        FAI_API.setConf("messenger", gParam.accountID, "", "lastUpdateBlock", latestBlock);
    };
    this.saveContacts = function () {
        FAI_API.writeFile("messenger", gParam.accountID, "contacts", JSON.stringify(contacts));
    }


    this.hasMessage = function (msg) {
        if (!messages[msg.IDForeign])
            return false;
        for (var j in messages[msg.IDForeign]) {
            if (messages[msg.IDForeign][j].txid == msg.txid && messages[msg.IDForeign][j].nVout == msg.nVout)
                return true;
        }
        return false;
    }
    this.notifiedTx = function (x) {
        //console.log(x);
        var data = FAI_API.getTxMessages(gParam.accountID, [x.tx.txid]);
        console.log(data);
        if (data && !data.error) {
            var msgs = [];
            for (var j in data) {
                msg = data[j];
                msgs.push(msg);
                this.addMessage(msg);
            }
            if (FAI_API.areIDsEqual(msg.IDFrom, currentContact) && FAI_API.areIDsEqual(msg.IDTo, gParam.accountID)) {
                this.decryptAndShow(currentContact, data);
                this.setUnreadLen(currentContact, 0);
            }
            if (FAI_API.areIDsEqual(msg.IDTo, currentContact)) 
                this.setUnreadLen(currentContact, 0);
        }
    };
    this.notifiedBlock = function (l) {
//        console.log(l);
        latestBlock = l.blockHeight;
        FAI_API.setConf("messenger", gParam.accountID, "", "lastUpdateBlock", latestBlock);
    }


    this.showModal = function (type) {
        var id = type + "-contact";
        if ($("#" + id).length <= 0) {
            var div = $("#modal-tpl").clone(true, true).attr("id", id);
            div.find(".cancel").unbind().click(function () {
                div.modal("hide");
            });
            if (type === "add") {
                div.find(".modal-header").find("h3").html(TR("Add New Contact"));
                div.find(".modal-body").find("p").html(TR("Please input the contact's account ID or domain name below:"));
                div.find(".modal-body").find("input").attr("placeholder", "FAIcoin Account ID").attr("name", "contact-id").addClass("input-id");
                div.find(".idbtn.cancel").html(TR("Cancel"));
                div.find(".idbtn.ok").html(TR("Add New")).unbind().click(function () {
                    console.log("add btin")
                    var id = $("#add-contact").find('input[name="contact-id"]').val();
                    if (contacts[id]) {
                        $("#add-contact").modal("hide");
                        CPage.showNotice(TR('ID already in contact list'));
                        return;
                    }
                    for (var iid in contacts) {
                        if (contacts[iid].domainname && contacts[iid].domainname == id) {
                            $("#add-contact").modal("hide");
                            CPage.showNotice(TR('Domain already in contact list'));
                            return;
                        }
                    }
                    if (id == accountID || (gParam.domain && gParam.domain.domain == id)) {
                        $("#add-contact").modal("hide");
                        CPage.showNotice(TR('can not add self to contact list'));
                        return;
                    }
                    if (!FAI_API.checkNameKey(id)) {
                        var dm = FAI_API.getDomainInfo(id);
                        console.log(dm);
                        if (!dm || dm.length == 0) {
                            CPage.showNotice(TR('ID is not valid'));
                            return;
                        }
                        if (!dm.forward || dm.forward.linkType != "ID") {
                            CPage.showNotice(TR('ID is not valid'));
                            return;
                        }
                        id = dm.forward.target;
                    }
                    if (Messenger.addContact(id)) {
                        CPage.showNotice(TR('ID successfully added to contact list'));
                        Messenger.saveContacts();
                    } else
                        CPage.showNotice(TR('add ID failed'));
                    $("#add-contact").modal("hide");
                });
            } else if (type === "edit") {
                div.find(".modal-header").find("h3").html(TR("Edit Contact"));
                div.find(".modal-body").html("").append($("#edit-contact-body-tpl").clone(true, true).children())
                $("#edit-contact-body-tpl").remove();
                div.find(".idbtn.cancel").html(TR("Close"));
                div.find(".modal-footer").find(".idbtn.ok").remove();
            }
            //doTranslate();
            $("body").append(div);
        }
        this.updateModal(type);
    };
    this.updateModal = function (type) {
        var id = type + "-contact";
        var div = $("#" + id);
        if (type === "edit") {
            div.find(".edit-modal-contact").html(currentContact);
            div.find("input[name='contact-alias']").val(contacts[currentContact].alias);
            var cc = contacts[currentContact].category;
            if (!cc)
                cc = "no category";
            div.find('.contact-category').html(TR(cc));
            switch (cc) {
                case "no category":
                    $("#btn-friend").text(TR('Add to friends'));
                    $("#btn-black").text(TR('Add_to_blacklist'));
                    break;
                case "friend":
                    $("#btn-friend").text(TR('Remove from Friends'));
                    $("#btn-black").text(TR('Add_to_blacklist'));
                    break;
                case "blacklisted":
                    $("#btn-friend").text(TR('Add to friends'));
                    $("#btn-black").text(TR('Remove from blacklist'));
            }

        }
        div.modal({show: true}).center();
    }
    this.getPageName = function () {
        var id = CUtil.getGet("chatto");
        if (id !== null)
            return "chatto";
        else
            return "messenger";
    }
}