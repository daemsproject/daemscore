/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
var DomainManager = new function () {
    var i = this;
    this.skip_init = false; //Set on sign up page
    var cVisible; //currently visible view    
    var accountID;
    var domains = [];
    var currentDomain="";
    var isInitialized = false;
    var language = 'en'; //Current language    
    var haveBoundReady = false;
    var cheques=[];
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

        console.log(msg);

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
        //console.log("switch to domain");
        updateCurrentDomainDisplay(domain);
        //$("#current-contact").html(id);
        $('#' + currentDomain.replace(".","_")).attr('class', '');
        $('#' + domain.replace(".","_")).attr('class', 'active');
        currentDomain = domain;
        //console.log(currentDomain);
    }
    function updateCurrentDomainDisplay(domain) {
        var d = domains[domain];
        //console.log(d);
        $("#home").find("input[name='domain-name']").val(d.domain);
        var t=BrowserAPI.getMatureTime(d.expireTime);
        //console.log(t);
        
        if (t.time==0)
            t="expired";
        else{
            t=t.time*1000+Number(new Date());
            //console.log(t);
            t=dateToString(new Date(t));
        }
        //console.log(t);
        $("#home").find("input[name='expire-time']").val(t);
        if(d.forward)
            $("#home").find("input[name='forward']").val(d.forward.target);
        else
            $("#home").find("input[name='forward']").val("");
        $("#home").find("input[name='alias']").val(d.alias);
        $("#home").find("input[name='icon']").val(d.icon);
        $("#home").find("input[name='intro']").val(d.intro);
    }
    function addDomain(d) {
        if (domains[d.domain])
            return;
        domains[d.domain] = d;
        //contacts[id].id=id;            
        var html = '<div id="' + d.domain.replace(".","_") + '" onclick="{DomainManager.switchToDomain(\'' + d.domain + '\')}" style="margin-top:20px">' + d.domain + '</div>';
        $("#domain-list").append(html);
    }
    this.readDomainInfo = function (domain) {
        return  BrowserAPI.getDomainInfo(domain);
    }
    this.getDomainsAll = function () {
        console.log("getDomains");
        var data = BrowserAPI.getDomainsByOwner(accountID);
        if (!data || data.error) {
            console.log("getMessage error");
            return;
        }
        console.log(data);
        for (var l in data) {
            addDomain(data[l]);
        }
    };

    function registerNotifications() {
        console.log("regnotifications");
        var ac = function (a) {
            console.log("refresh");
            window.location.href = window.location.href;
        }
        //BrowserAPI.regNotifyBlocks(aa);        
        //BrowserAPI.regNotifyTxs(ab,[accountID]);
        BrowserAPI.regNotifyAccount(ac);
        console.log("regnotifications success");
//        BrowserAPI.regNotifyPeers(this.notifiedPeers);
    }
    function bindReady() {
        if (haveBoundReady) {
            return;
        }
        haveBoundReady = true;
        $("#btn-reg").unbind().click(function () {
           $("#reg-domain").modal({backdrop: "static", show: true});           
           $("#reg-domain").center();
           
           //cheques=BrowserAPI.getUnspent(accountID);
        });
        $("#btn-renew").unbind().click(function () {   
            console.log(domains[currentDomain].group);
            $("#value-needed-renew").text(domains[currentDomain].group); 
            $("#domain-to-renew").text(currentDomain); 
            $("#renew-domain").modal({backdrop: "static", show: true});           
            $("#renew-domain").center();           
        });
        $("#btn-transfer").unbind().click(function () { 
            $("#domain-to-transfer").text(currentDomain); 
            $("#transfer-domain").modal({backdrop: "static", show: true});           
            $("#transfer-domain").center();           
        });
        $("#reg-domain").find(".btn-secondary").unbind().click(function () {
            $("#reg-domain").modal("hide");
        });
        $("#reg-domain").find(".btn-primary").unbind().click(function () {
            //TODO check inputs
            var locktime = $("#reg-domain").find("input[name='lock-time']").val();
            console.log(accountID);
            BrowserAPI.registerDomain(accountID, $("#reg-domain").find("input[name='domain-name']").val(), locktime*3600*24+Math.ceil((new Date())/1000),
                    function (a) {
                        i.makeNotice('success', 'reg-domain-success', "Domain register sent!Please check after the tx is confirmed");
                        $("#reg-domain").modal("hide");
                    },
                    function (e) {
                        i.makeNotice('error', 'reg-domain-error', e);
                    });
        });
        $("#btn-check-domain").unbind().click(function () {
            //TODO check level2 domain, 
            var domain = $("#reg-domain").find("input[name='domain-name']").val();
            if(IsLevel2Domain(domain)){               
                var level1=GetLevel1Domain(domain);
                console.log(level1);
                if(!domains[level1]){
                    i.makeNotice('error', 'check-domain-error', 'level1 domain is not owned by account');
                   return;           
               }
                $("#value-needed").html(100);
            }else{
            //console.log(domain.substring(domain.length-2));            
                if (domain.substring(domain.length-2) == ".f")
                    $("#value-needed").html(10000);
                else if (domain.substring(domain.length-4) == ".fai")
                    $("#value-needed").html(100);
                else {
                    i.makeNotice('error', 'check-domain-error', 'wrong domain format');
                    return;
                }
            }
            var d = BrowserAPI.getDomainInfo(domain);
            console.log(d);
            //console.log(BrowserAPI.getMatureTime(d.expireTime));
            //console.log((new Date())/1000);
            
            if (d.domain&&d.expireTime){
               var timeleft= BrowserAPI.getMatureTime(d.expireTime);
               if(timeleft.time>=(new Date())/1000){
                    i.makeNotice('error', 'check-domain-error', 'domain already exists');
                    return;
                }
            }
            
            i.makeNotice('success', 'check-domain-success', 'domain is available!');
                
        });
        $("#transfer-domain").find(".btn-secondary").unbind().click(function () {
            $("#transfer-domain").modal("hide");
        });
        $("#transfer-domain").find(".btn-primary").unbind().click(function () {
            //TODO check inputs            
            console.log(accountID);
            var idto=$("#transfer-domain").find("input[name='transfer-id']").val();
            BrowserAPI.transferDomain(accountID, currentDomain, idto,
                    function (a) {
                        i.makeNotice('success', 'transfer-domain-success', "Domain transfer sent!Please check after the tx is confirmed");
                        $("#transfer-domain").modal("hide");
                    },
                    function (e) {
                        i.makeNotice('error', 'transfer-domain-error', e);
                    });
        });
        $("#renew-domain").find(".btn-secondary").unbind().click(function () {
            $("#renew-domain").modal("hide");
        });
        $("#renew-domain").find(".btn-primary").unbind().click(function () {
            //TODO check inputs
            var locktime = $("#renew-domain").find("input[name='lock-time']").val();
            console.log(accountID);
            BrowserAPI.renewDomain(accountID, currentDomain, locktime*3600*24+Math.ceil((new Date())/1000),
                    function (a) {
                        i.makeNotice('success', 'renew-domain-success', "Domain renew sent!Please check after the tx is confirmed");
                        $("#renew-domain").modal("hide");
                    },
                    function (e) {
                        i.makeNotice('error', 'reg-domain-error', e);
                    });
        });
        $("#btn-update").unbind().click(function () {
            console.log("update clicked");
            var changed = false;
            var d = {};
            //d.domain = currentDomain;
            var alias = $("#home").find("input[name='alias']").val();
            if (alias != domains[currentDomain].alias) {
                d.alias = alias;
                changed = true;
            }
            var forward = $("#home").find("input[name='forward']").val();
            if (!domains[currentDomain].forward||!domains[currentDomain].forward.target||forward != domains[currentDomain].forward.target) {
                d.forward = forward;
                changed = true;
            }
            var icon = $("#home").find("input[name='icon']").val();
            if (icon != domains[currentDomain].icon) {
                d.icon = icon;
                changed = true;
            }
            var intro = $("#home").find("input[name='intro']").val();
            if (intro != domains[currentDomain].intro) {
                d.intro = intro;
                changed = true;
            }
            if (changed) {
                console.log(d);
                BrowserAPI.updateDomain(accountID, currentDomain, d, function (newd) {
                    console.log(newd);
                    //domains[currentDomain] = newd;
                     i.makeNotice('success', 'update-domain-success', 'domain updated!Please wait for 1 confirmation to validate update.');
                }, function (e) {
                    i.makeNotice('error', 'update-domain-error', e);
                });
            }else{
                i.makeNotice('error', 'update-domain-error', 'nothing to update');
            }
        });
    }

    function initAccount() {
        accountID = BrowserAPI.getAccountID();
        //$("#account-id").html(accountID);
        //IDs=BrowserAPI.getIDs(accountID);  
        //BrowserAPI.getNewID(accountID);
        registerNotifications();
        //readContacts(Messenger.get_history);    
        i.getDomainsAll();
    }


    $(document).ready(function () {
        if (!$.isEmptyObject({}) || !$.isEmptyObject([])) {
            i.makeNotice('error', 'error', 'Object.prototype has been extended by a browser extension. Please disable this extensions and reload the page.');
            return;
        }
        //Listener to reload the page on App Cache update
        window.applicationCache.addEventListener('updateready', function () {
            if (window.applicationCache.status === window.applicationCache.UPDATEREADY) {
                window.applicationCache.swapCache();
                location.reload();
            }
        });
        //Disable autocomplete in firefox
        $("input,button,select").attr("autocomplete", "off");
        var body = $(document.body);
        bindInitial();
        initAccount();
        bindReady();        
        //Frame break
        if (top.location != self.location) {
            top.location = self.location.href
        }
        $(document).ajaxStart(function () {
            setLogoutImageStatus('loading_start');

            $('.loading-indicator').fadeIn(200);
        }).ajaxStop(function () {
            setLogoutImageStatus('loading_stop');

            $('.loading-indicator').hide();
        });
    });
}