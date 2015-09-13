var Settings = new function () {
    var i = this;
    var accountID;
    var settings;
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
//        if (msg == null || msg.length == 0)
//            return;
//        var el = $('<div class="alert alert-block alert-' + type + '"></div>');
//        el.text('' + msg);
//        if ($('#' + id).length > 0) {
//            el.attr('id', id);
//            return;
//        }
//        $("#notices").append(el).hide().fadeIn(200);
//        (function () {
//            var tel = el;
//            setTimeout(function () {
//                tel.fadeOut(250, function () {
//                    $(this).remove();
//                });
//            }, timeout ? timeout : 5000);
//        })();
        console.log(msg);
        CPage.showNotice(msg, timeout);
    }
    this.updateDomain = function (name, reset) {
        console.log(name);
        var html = "input[name='" + name + "']";
        var value = $(html).val();
        console.log(value);
        if (reset)
            value = "";
        else {
            if (!value || value == settings.pagedomains[name])
                return;
        }
        console.log(2);
        var r = FAI_API.updateSettings("pagedomain", name, value);
        console.log(typeof r);
        if (r === true)
            this.makeNotice("success", "domain", TR("update success"));
        else
            this.makeNotice("error", "domain", TR("update failed"));
        LoadSettings();
    }
    this.updateService = function (name) {
        var value = $("#" + name).html() == TR("ON") ? "false" : "true";
        FAI_API.updateSettings("serviceflages", name, value);
        LoadSettings();
    }
    this.updateLanguage = function () {
        var value = $("select[name='language']").val();
        var r = FAI_API.setLang(value);
        if (r === true)
            this.makeNotice("success", "lang", TR("update success"));
        else
            this.makeNotice("error", "lang", TR("update failed"));
    }
    this.updateTimeOut = function () {
        var value = $("input[name='timeout']").val();
        if (isNaN(value))
            this.makeNotice("error", "timeout", TR('value is not number'));
        if (value < 1)
            this.makeNotice("error", "timeout", TR('value is too small'));
        var r = FAI_API.updateSettings("filepackagetimeout", "", value * 1000);
        //      console.log(r);
        if (r == true)
            this.makeNotice("success", "cache", TR("update success"));
        else
            this.makeNotice("error", "cache", TR("update failed"));
    }
    this.updateMaxCache = function () {
        var value = $("input[name='maxcache']").val();
        if (isNaN(value))
            this.makeNotice("error", "maxcache", TR('value is not number'));
        if (value < 100)
            this.makeNotice("error", "maxcache", TR('value is too small'));
        var r = FAI_API.updateSettings("maxcachesize", "", value);

        if (r === true)
            this.makeNotice("success", "cache", TR("update success"));
        else
            this.makeNotice("error", "cache", TR("update failed"));
    }
    this.clearCache = function () {
        var r = FAI_API.clearCache();
        if (r === true)
            this.makeNotice("success", "cache", TR("clear success"));
        else
            this.makeNotice("error", "cache", TR("clear failed"));
    }
    function LoadSettings()
    {
        settings = FAI_API.getSettings();
        var html = "<tr><th>" + TR('Application') + "</th><th>" + TR('Default Domain Name') + "</th><th>" + TR('Action') + "</th></tr>";
        for (var name in settings.pagedomains) {
            html += "<tr><td>" + TR(name) + "</td>";
            html += '<td><input  name="' + name + '"   value="' + TR(settings.pagedomains[name]) + '"/></td>';
            html += '<td>';
            html += '<button onclick="Settings.updateDomain(' + "'" + name + "'" + ')">' + TR('Change') + '</button>';
            html += '<button onclick="Settings.updateDomain(' + "'" + name + "'" + ',1)">' + TR('default') + '</button>';
            html += '</td></tr>';
        }
        $("#table_pagedomains").html(html);
        html = "<tr><th>" + TR('Service') + "</th><th>" + TR('Status') + "</th><th>" + TR('Action') + "</th></tr>";
        var name = "full_node_plus_service";
        html += "<tr><td>" + TR(name) + "</td>";
        var status = (settings.serviceflags & 8) ? TR("ON") : TR("OFF");
        html += '<td  id="' + name + '">' + status + '</td>';
        html += '<td>';
        html += '<button onclick="Settings.updateService(' + "'" + name + "'" + ')">' + TR('Change') + '</button>';
        html += '</td></tr>';
        $("#table_services").html(html);
        html = '<div  style="float:left">' + TR('Language') + ':</div>';
        html += "<select name='language'><option value='en'>English</option><option value='zh_CN' >中文</option></select> ";
        html += '<button onclick="Settings.updateLanguage()">' + TR('Change') + '</button>';
        $("#language-settings").html(html);
        var lang = FAI_API.getLang();
        if (lang.userlang)
            lang = lang.userlang;
        else
            lang = lang.systemlang;
        $("select[name='language']").val(lang);
        html = "<tr><th>" + TR("Item") + "</th><th>" + TR('Settings') + "</th><th>" + TR('Action') + "</th></tr>";
        html += "<tr><td>" + TR('page download timeout') + "</td>";
        html += '<td><input  name="timeout"   value="' + settings.filepackagetimeout / 1000 + '"/>' + TR('seconds') + '</td>';
        html += '<td>';
        html += '<button onclick="Settings.updateTimeOut()">' + TR('Change') + '</button>';
        html += '</td></tr>';
        html += "<tr><td>" + TR('Cache') + "</td>";
        html += '<td></td>';
        html += '<td>';
        html += '<button onclick="Settings.clearCache()">' + TR('Clear Cache') + '</button>';
        html += '</td></tr>';
        $("#table_cache").html(html);
    }
    function bindReady() {
        if (haveBoundReady) {
            return;
        }
        haveBoundReady = true;
        LoadSettings();
    }
    function initAccount() {
        accountID = FAI_API.getAccountID();
    }
    $(document).ready(function () {
        $("#tpls").load("templates.html", function () {
            CUtil.initGParam();
            doTranslate();
            $("input,button,select").attr("autocomplete", "off");
            bindInitial();
            initAccount();
            bindReady();
            CPage.prepareNotice("setting");
            CPage.registerNotifications();
        });
    });
}