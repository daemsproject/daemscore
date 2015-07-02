/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
var Shop = new function () {
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
    this.initAddNew=function()
    {
        //console.log("add new clicked");
         $("#add-new").find('input[name="product-id"]').val(parseInt(Math.random()*256*256*256*256));
    }
    function bindReady() {
        if (haveBoundReady) {
            return;
        }
        haveBoundReady = true;
         //console.log("bindready");       
        
        $("#btn-add-new").unbind().click(function () {            
            var d = {};
            //d.domain = currentDomain;
            d.productid=$("#add-new").find("input[name='product-id']").val();
            var productname = $("#add-new").find("input[name='product-name']").val();            
            if(!productname){
                    i.makeNotice('error', 'publish-product-error', "Empty product name");
                    return 
            }
            d.productname=productname;
            var price= $("#add-new").find("input[name='price']").val();            
            if(isNaN(price)){
                i.makeNotice('error', 'publish-product-error', "invalid price");
                    return 
            }
            d.price=price;
            var shipmentfee= $("#add-new").find("input[name='shipment-fee']").val();            
            if(shipmentfee){
                if(isNaN(shipmentfee)){
                    i.makeNotice('error', 'publish-product-error', "invalid shipment fee");
                    return 
                }
                d.shipmentfee=shipmentfee;
            }            
            var recipient= $("#add-new").find("input[name='recipient']").val();   
            if(recipient)
                d.recipient=recipient;
            var expiretime= $("#add-new").find("input[name='expire-time']").val();               
            if(expiretime){
                if(isNaN(expiretime)||expiretime>3650||expiretime<0){
                    i.makeNotice('error', 'publish-product-error', "invalid expire time");
                    return 
                }
                expiretime=expiretime*3600*24+Math.ceil((new Date())/1000);
                d.expiretime=expiretime;                                          
            }else
                expiretime=0;
            var icon = $("#add-new").find("input[name='icon']").val();            
            if(icon)
                d.icon=icon;
            var intro = $("#add-new").find("textarea[name='intro']").val();            
            if(intro)
                d.intro=intro;
            var tags = $("#add-new").find("textarea[name='tags']").val();   
            console.log(tags);
            console.log(tags.split(","));
            if(tags)
                d.tags=tags.split(",");
            BrowserAPI.publishProduct(accountID, d,expiretime, function (data) {
                    console.log(data);                    
                     i.makeNotice('success', 'publish-product-success', 'Product published!');
                }, function (e) {
                    i.makeNotice('error', 'publish-product-error', e);
                });            
        });
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