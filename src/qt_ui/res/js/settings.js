/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
var Settings = new function () {
    var i = this;
    this.skip_init = false; //Set on sign up page
    var cVisible; //currently visible view    
    var accountID;
    var settings;
    var isInitialized = false;
    var language = 'en'; //Current language    
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
    this.updateDomain = function (name, reset) {
        console.log("update:" + name);
        var html = "input[name='" + name + "']";
        //console.log(html);
        var value = $(html).val();
        if (reset)
            value = "";
        else {
            console.log(value);
            if (!value || value == settings.pagedomains[name])
                return;
        }
        this.makeNotice("domain", "", BrowserAPI.updateSettings("pagedomain", name, value));
        LoadSettings();
    }
    this.updateService = function (name) {         
        var value=$("#"+name).html()=="ON"?"false":"true";           
        console.log(value);
        this.makeNotice("domain", "", BrowserAPI.updateSettings("serviceflages", name, value));
        LoadSettings();
    }
    this.updateLanguage = function () {
        console.log("updateLanguage");
        var value=$("select[name='language']").val();
        console.log(value);
        this.makeNotice("domain", "", BrowserAPI.updateSettings("language", "", value));
    }
    function registerNotifications() {
        console.log("regnotifications");
        var ac = function (a) {
            console.log("refresh");
            window.location.href = window.location.href;
        }
        //BrowserAPI.regNotifyBlocks(aa);        
        //BrowserAPI.regNotifyTxs(ab,[accountID]);
        //BrowserAPI.regNotifyAccount(ac);
        console.log("regnotifications success");
//        BrowserAPI.regNotifyPeers(this.notifiedPeers);
    }
    function LoadSettings()
    {
        settings = BrowserAPI.getSettings();
        var html = "<tr><th>Application</th><th>Default Domain Name</th><th>Action</th></tr>";
        for (var name in settings.pagedomains) {
            html += "<tr><td>" + name + "</td>";
            html += '<td><input  name="' + name + '"   value="' + settings.pagedomains[name] + '"/></td>';
            html += '<td>';
            html += '<button onclick="Settings.updateDomain(' + "'" + name + "'" + ')">change</button>';   
            html += '<button onclick="Settings.updateDomain(' + "'" + name + "'" + ',1)">default</button>';
            html += '</td></tr>';
        }
        //console.log(html); 
        $("#table_pagedomains").html(html);
        html = "<tr><th>Service</th><th>Status</th><th>Action</th></tr>";
        var name = "full_node_plus_service";
        html += "<tr><td>"+name+"</td>";
        
        var status=(settings.serviceflags & 8) ? "ON" : "OFF";
        html += '<td  id="' + name + '">'+status+'</td>';
        html += '<td>';
        html += '<button onclick="Settings.updateService(' + "'" + name + "'" + ')">change</button>';        
        html += '</td></tr>';

        //console.log(html); 
        $("#table_services").html(html);
        html = '<div  style="float:left">Language:</div>';
        html += "<select name='language'><option value='en' selected>English</option><option value='zh_CN' >中文</option></select> ";
        html += '<button onclick="Settings.updateLanguage()">change</button>';
        //console.log(html); 
        $("#language-settings").html(html);
    }


    function bindReady() {
        if (haveBoundReady) {
            return;
        }
        haveBoundReady = true;
        //console.log("bindready");       
        
        //console.log(settings);
        LoadSettings();


    }

    function initAccount() {
        accountID = BrowserAPI.getAccountID();
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
        registerNotifications();
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