var DomainManager = new function () {
    var i = this;
    var accountID;
    var domains = [];
    var currentDomain = "";
    var haveBoundReady = false;
    function bindInitial() {
        $('.modal').on('show', function () {
            hidePopovers();
            $(this).center();
        }).on('hidden', function () {
            var visible = $('.modal:visible');
            var notices = $('#notices').remove();
            if (visible.length > 0)
                visible.find('.modal-body').prepend(notices);
            else
                $('#main-notices-container').append(notices);

        }).on('shown', function () {
            hidePopovers();
            var self = $(this);
            setTimeout(function () {
                if (self.is(':visible')) {
                    self.find('.modal-body').prepend($('#notices').remove());
                }
            }, 100);

            self.center();
        });
    }
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
    this.switchToDomain = function (domain) {
        updateCurrentDomainDisplay(domain);
        $('#' + currentDomain.replace(".", "_")).attr('class', '');
        $('#' + domain.replace(".", "_")).attr('class', 'active');
        currentDomain = domain;
    }
    function updateCurrentDomainDisplay(domain) {
        var d = domains[domain];
        $("#home").find("input[name='domain-name']").val(d.domain);
        var t = FAI_API.getMatureTime(d.expireTime);
        if (t.time == 0)
            t = TR("expired");
        else {
            t = t.time * 1000 + Number(new Date());
            t = CUtil.dateToString(new Date(t));
        }
        $("#home").find("input[name='expire-time']").val(t);
        if (d.forward)
            $("#home").find("input[name='forward']").val(d.forward.target);
        else
            $("#home").find("input[name='forward']").val("");
        $("#home").find("input[name='alias']").val(d.alias);
        $("#home").find("input[name='icon']").val(d.icon);
        $("#home").find("input[name='intro']").val(d.intro);
        if (d.tags){
            $("#home").find("input[name='tags']").val(d.tags.join(","));
        }
    }
    function addDomain(d) {
        if (domains[d.domain])
            return;
        domains[d.domain] = d;
        var html = '<div id="' + d.domain.replace(".", "_") + '" onclick="{DomainManager.switchToDomain(\'' + d.domain + '\')}" style="margin-top:20px">' + d.domain + '</div>';
        $("#domain-list").append(html);
    }
    this.readDomainInfo = function (domain) {
        return  FAI_API.getDomainInfo(domain);
    }
    this.getDomainsAll = function () {
        var data = FAI_API.getDomainsByOwner(accountID);
        if (!data || data.error) {
            console.log("getMessage error");
            return;
        }
        for (var l in data) {
            addDomain(data[l]);
        }
    };
    function getForwardSig() {
        var id = $("#home").find("input[name='forward']").val();
        var msg4sig = currentDomain + "->" + id;
        $("#home").find("input[name='forward_msg4sig']").val(msg4sig);
        var sig = FAI_API.signMessage(id, msg4sig);
        if (!sig.signature || sig.signature.length != 88)
            return false;
        $("#home").find("input[name='forward_sig']").val(sig.signature);
        return true;
    }
    function checkForwardSig() {
        var id = $("#home").find("input[name='forward']").val();
        var msg4sig = currentDomain + "->" + id;
        var sig = $("#home").find("input[name='forward_sig']").val();
        return FAI_API.verifyMessage(id, sig, msg4sig);
    }

    function registerNotifications() {
        var aa = function (a) {
            CPage.notifyBlock(a);
        };
        var ab = function (a) {
            CPage.updateBalance();
        };
        var ac = function (a) {
            window.location.href = window.location.href;
        }
        FAI_API.regNotifyTxs(ab, [accountID]);
        FAI_API.regNotifyAccount(ac);
         FAI_API.regNotifyBlocks(aa);
    }
    this.getDomainLockValue = function (domain) {
        if (domain.substring(domain.length - 2) == ".f")
            return 10000;
        else if (domain.substring(domain.length - 4) == ".fai")
            return 100;
        else {
            i.makeNotice('error', 'check-domain-error', TR('wrong domain format'));
            return 0;
        }
    }

    function bindReady() {
        if (haveBoundReady) {
            return;
        }
        haveBoundReady = true;
        $("#dateinput").datepicker(CPage.initDatePickerOptions());
        $("#renew-dateinput").datepicker(CPage.initDatePickerOptions());
        $("#btn-reg").unbind().click(function () {
            $("#reg-domain").modal({backdrop: "static", show: true});
            $("#reg-domain").center();
        });
        $("#btn-renew").unbind().click(function () {
            if (typeof domains[currentDomain] === "undefined") {
                CPage.showNotice(TR("Please choose a domain first"));
                return;
            }
            $("#value-needed-renew").text(domains[currentDomain].group);
            $("#renew-domain").find("input[name='value-to-lock']").val(domains[currentDomain].group);
            $("#domain-to-renew").text(currentDomain);
            $("#renew-domain").modal({backdrop: "static", show: true});
            $("#renew-domain").center();
        });
        $("#btn-transfer").unbind().click(function () {
            if (IsLevel2Domain(currentDomain)) {
                i.makeNotice('error', 'reg-domain-error', TR("subdomain can't be transferred"));
                return;
            }
            $("#domain-to-transfer").text(currentDomain);
            $("#transfer-domain").modal({backdrop: "static", show: true});
            $("#transfer-domain").center();
        });
        $("#reg-domain").find(".cancel").unbind().click(function () {
            $("#reg-domain").modal("hide");
        });
        $("#reg-domain").find(".ok").unbind().click(function () {
            var locktime = Math.ceil($("#dateinput").datepicker("getDate") === null ? 0 : $("#dateinput").datepicker("getDate").getTime() / 1000 + 86400); 
            if (isNaN(locktime) || locktime < (new Date().getTime())/1000+86400) {
                i.makeNotice('error', 'reg-domain-error', TR("lock time is too short"));
                return;
            }
            var feerate = FAI_API.getFeeRate(0.15);
            var domainname = $("#reg-domain").find("input[name='domain-name']").val();
            if (!IsValidDomain(domainname)) {
                i.makeNotice('error', 'check-domain-error', TR('invaid domain format'));
                return;
            }
            var lockvalue = i.getDomainLockValue(domainname);
            if (lockvalue == 0)
                return;
            var lv2 = Number($("#reg-domain").find("input[name='value-to-lock']").val());
            if (lockvalue < lv2)
                lockvalue = lv2;
            FAI_API.registerDomain(accountID, domainname, lockvalue * 1000000, locktime, feerate,
                    function (a) {
                        i.makeNotice('success', 'reg-domain-success', TR("Domain register sent!Please check after the tx is confirmed"));
                        $("#reg-domain").modal("hide");
                    },
                    function (e) {
                        i.makeNotice('error', 'reg-domain-error', TR(e));
                    });
        });
        $("#btn-check-domain").unbind().click(function () {
            var domain = $.trim($("#reg-domain").find("input[name='domain-name']").val());
            if (!IsValidDomain(domain)) {
                i.makeNotice('error', 'check-domain-error', TR('invaid domain format'));
                return;
            }

            if (IsLevel2Domain(domain)) {
                var level1 = GetLevel1Domain(domain);
                if (!domains[level1]) {
                    i.makeNotice('error', 'check-domain-error', TR('level1 domain is not owned by account'));
                    return;
                }
                $("#value-needed").html(100);
            } else {
                if (domain.substring(domain.length - 2) == ".f") {
                    $("#value-needed").html(10000);
                    $("#reg-domain").find("input[name='value-to-lock']").val(10000);
                }
                else if (domain.substring(domain.length - 4) == ".fai") {
                    $("#value-needed").html(100);
                    $("#reg-domain").find("input[name='value-to-lock']").val(100);
                }
                else {
                    i.makeNotice('error', 'check-domain-error', TR('wrong domain format'));
                    return;
                }
            }
            var d = FAI_API.getDomainInfo(domain);
            if (d.domain && d.expireTime) {
                var timeleft = FAI_API.getMatureTime(d.expireTime);
                if (timeleft.time > 0) {
                    i.makeNotice('error', 'check-domain-error', TR('domain already exists'));
                    return;
                }
            }
            i.makeNotice('success', 'check-domain-success', TR('domain is available!'));
        });
        $("#transfer-domain").find(".cancel").unbind().click(function () {
            $("#transfer-domain").modal("hide");
        });
        $("#transfer-domain").find(".ok").unbind().click(function () {
            var idto = $.trim($("#transfer-domain").find("input[name='transfer-id']").val());
            var feerate = FAI_API.getFeeRate(0.15);
            FAI_API.transferDomain(accountID, currentDomain, idto, feerate,
                    function (a) {
                        i.makeNotice('success', 'transfer-domain-success', TR("Domain transfer sent!Please check after the tx is confirmed"));
                        $("#transfer-domain").modal("hide");
                    },
                    function (e) {
                        i.makeNotice('error', 'transfer-domain-error', TR(e));
                    });
        });
        $("#renew-domain").find(".cancel").unbind().click(function () {
            $("#renew-domain").modal("hide");
        });
        $("#renew-domain").find(".ok").unbind().click(function () {
            var locktime = Math.ceil($("#renew-dateinput").datepicker("getDate") === null ? 0 : $("#renew-dateinput").datepicker("getDate").getTime() / 1000 + 86400); 
            if (isNaN(locktime) || locktime < 1) {
                i.makeNotice('error', 'reg-domain-error', TR("lock time is too short"));
                return;
            }
            var feerate = FAI_API.getFeeRate(0.15);
            var lockvalue = i.getDomainLockValue(currentDomain);
            if (lockvalue == 0)
                return;
            var lv2 = Number($("#renew-domain").find("input[name='value-to-lock']").val());
            if (lockvalue < lv2)
                lockvalue = lv2;
            FAI_API.renewDomain(accountID, currentDomain, lockvalue * 1000000, locktime, feerate,
                    function (a) {
                        i.makeNotice('success', 'renew-domain-success', TR("Domain renew sent!Please check after the tx is confirmed"));
                        $("#renew-domain").modal("hide");
                    },
                    function (e) {
                        i.makeNotice('error', 'reg-domain-error', TR(e));
                    });
        });
        $("#btn-update").unbind().click(function () {
            if (typeof domains[currentDomain] === "undefined") {
                CPage.showNotice(TR("Please choose a domain before updating"))
                return;
            }
            var changed = false;
            var d = {};
            var alias = $.trim($("#home").find("input[name='alias']").val());
            if (alias != domains[currentDomain].alias) {
                d.alias = alias;
                changed = true;
            }
            var forward = $.trim($("#home").find("input[name='forward']").val());
            var forwardmsg4sig = $("#home").find("input[name='forward_msg4sig']").val();
            if (forward.length > 0 && (!domains[currentDomain].forward || !domains[currentDomain].forward.target || forward != domains[currentDomain].forward.target)) {
                d.forward = forward;
                if (getStrLinkType(forward) == "id") {
                    if (!checkForwardSig()) {
                        if (!getForwardSig() || !checkForwardSig())
                        {
                            i.makeNotice('error', 'update-domain-error', TR("please sign with the forwarded ID and paste the signature in 'Forward to ID sig' cell"));
                            return;
                        }
                    }
                    var forwardsig = $("#home").find("input[name='forward_sig']").val();
                    d.forwardsig = forwardsig;
                }
                changed = true;
            }
            var icon = $.trim($("#home").find("input[name='icon']").val());
            if (icon != domains[currentDomain].icon) {
                d.icon = icon;
                changed = true;
            }
            var intro = $.trim($("#home").find("input[name='intro']").val());
            if (intro != domains[currentDomain].intro) {
                d.intro = intro;
                changed = true;
            }
            var tags = $.trim($("#home").find("input[name='tags']").val());
            if (tags && (tags != domains[currentDomain].tags.join(","))) {
                changed = true;
                d.tags = tags.split(",");
            }
            if (changed) {
                var feerate = FAI_API.getFeeRate(0.15);
                FAI_API.updateDomain(accountID, currentDomain, d, feerate, function (newd) {
                    i.makeNotice('success', 'update-domain-success', TR('domain updated!Please wait for 1 confirmation to validate update.'));
                }, function (e) {
                    i.makeNotice('error', 'update-domain-error', TR(e));
                });
            } else {
                i.makeNotice('error', 'update-domain-error', TR('nothing to update'));
            }
        });
        $("#home").find("input[name='forward']").unbind().change(function () {
            var forward = $("#home").find("input[name='forward']").val();
            var dF = FAI_API.b32CheckDecode(forward);
            if (dF && forward !== gParam.accountID && currentDomain) {
                getForwardSig();
                $(".dmn-sig").removeClass("hide");
                $("#cpy-msg").removeClass("hide");
            } else {
                $(".dmn-sig").addClass("hide");
                $("#cpy-msg").addClass("hide");
            }
        });
        $("#cpy-msg").click(function () {
            var msg = $(this).parent().find("input[name='forward_msg4sig']").val();
            CUtil.copyToClipboard(msg);
        });
    }
    function initAccount() {
        accountID = FAI_API.getAccountID();
        registerNotifications();
        i.getDomainsAll();
    }
    $(document).ready(function () {
        $("#tpls").load("templates.html", function () {
            CUtil.initGParam();
            doTranslate();
            $("input,button,select").attr("autocomplete", "off");
            bindInitial();
            initAccount();
            bindReady();
            CPage.prepareNotice("domain");
            CPage.updateBalance();
            CPage.updateCblc();
        });
    });
}