/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
var Tools = new function () {
    var i = this;
    var accountID;
    var haveBoundReady = false;
    function hidePopovers() {
        try {
            $('.popover').remove();
        } catch (e) {
        }
    }
    this.makeNotice = function (type, id, msg, timeout) {

        if (msg == null || msg.length == 0)
            return;
        var el = $('<div class="alert alert-block alert-' + type + '"></div>');

        el.text('' + msg);

        if ($('#' + id).length > 0) {
            el.attr('id', id);
            return;
        }

        $("#notices").append(el).hide().fadeIn(200);

        (function () {
            var tel = el;
            setTimeout(function () {
                tel.fadeOut(250, function () {
                    $(this).remove();
                });
            }, timeout ? timeout : 5000);
        })();
    }
    function getSig() {
        var msg4sig = $('#sign-message').find("textarea[name='message']").val();
        var sig = FAI_API.signMessage(accountID, msg4sig);
        if (!sig.signature || sig.signature.length != 88)
            return false;
        $("#sign-message").find("textarea[name='signature']").val(sig.signature);
        return true;
    }
    function verifySig() {
        var id = $("#verify-message").find("input[name='verify-id']").val();
        var msg4sig = $("#verify-message").find("textarea[name='message']").val();
        var sig = $("#verify-message").find("textarea[name='signature']").val();
        if (FAI_API.verifyMessage(id, sig, msg4sig)) {
            i.makeNotice('success', 'verify-message-success', TR('Verify Signature Passed'));
        }
        else {
            i.makeNotice('error', 'verify-message-error', TR('Verify Signature failed'));
        }
    }
    function showPeers() {
        var datajson = [];
        var peerinfo = FAI_API.icall("getpeerinfo", datajson);
        var html = "";
        for (var i in peerinfo)
            html += peerinfo[i].addr + "</br>";
        $("#show-peers").html(html);
    }
    function addPeers() {
        var html = TR("Peer:") + '<input  name="add-peer"   placeholder="' + TR('Input peer info to add') + '"/>';
        html += '<button onclick="Tools.addNode()">' + TR('Add') + '</button>';
        $("#add-peers").html(html);
    }
    this.addNode = function () {
        var peer = $("#add-peers").find("input[name='add-peer']").val();
        var r = FAI_API.icall("addnode", [peer, "add"]);
        //console.log(r);

        if (!r || r.error) {
            i.makeNotice('error', 'addNode-error', TR("Add node failed"));
        }
        else {
            i.makeNotice('success', 'addNode-success', TR("Add node success"));
        }
    }
    function installPackages()
    {
        var html = "<tr><th>" + TR('Package Name') + "</th><th>" + TR('Size') + "</th><th>" + TR('Action') + "</th></tr>";
        var d = FAI_API.getDomainInfo("installpackages.f");
        console.log(d);
        var p = {};
        p.frAddrs = [d.forward.target];
        p.withcc = ["CC_FILE_PACKAGE_P"];
        p.cformat = 5;
        var list = FAI_API.icall("getcontentsbyaddresses", [p]);
        console.log(list);
        for (var i in list) {
            var package = list[i].content[0].content;
            var packagename;
            var size;
            for (var j in package) {
                if (package[j].cc_name == "CC_NAME")
                    packagename = package[j].content;
                if (package[j].cc_name == "CC_FILE_PACKAGE_SIZE")
                    size = package[j].content;
            }
            html += "<tr><td>" + packagename + "</td>";
            html += '<td>' + size + '</td>';
            html += '<td>';
            html += '<button onclick="window.location.assign(' + "'" + list[i].link + "'" + ')">' + TR('Install') + '</button>';
            html += '</td></tr>';
        }
    }
    function registerNotifications() {
        var ac = function (a) {
            window.location.href = window.location.href;
        }
        FAI_API.regNotifyAccount(ac);
    }
    function bindReady() {
        if (haveBoundReady) {
            return;
        }
        haveBoundReady = true;
        $("input[name='id']").val(accountID);
        $("#btn-verify-message").unbind().click(function () {
            verifySig();
        });
        $("#btn-sign-message").unbind().click(function () {
            getSig();
        });
        $("#btn-show-peers").click(function () {
            showPeers();
        });
        $("#btn-add-peers").click(function () {
            addPeers();
        });
        $("#btn-install-packages").click(function () {
            installPackages();
        });
    }
    function initAccount() {
        accountID = FAI_API.getAccountID();
        registerNotifications();
    }
    $(document).ready(function () {
        $("#tpls").load("templates.html", function () {
            CUtil.initGParam();
            doTranslate();
            $("input,button,select").attr("autocomplete", "off");
            initAccount();
            bindReady();
            CPage.prepareNotice("wallet");
            CPage.updateBalance();
            CPage.updateCblc();
            CPage.registerNotifications();
        });
    });
}