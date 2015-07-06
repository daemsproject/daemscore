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
    var pagelen=100;
    var page=0;
    var products=[];
    var currentProduct;
    var cart={};
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
    this.showViewProduct=function(tags){
        products=BrowserAPI.searchProducts(tags,pagelen,page*pagelen);        
        console.log(products);
        if(!products||products.error||products.length==0){            
            $('#buy-product-list').html("<tr><td>no products found, please change search tags and retry</td></tr>");
            return;
        }
        var html="<tr><th>Icon</th><th>Product Name</th><th>Price</th><th>Seller</th><th>Action</th></tr>";
        for(var j in products){
            html+="<tr><td>";
            var p=products[j];
            if (p.icon){
                html+=CBrowser.createIconHtml(CBrowser.getB64DataFromLink(p.icon));
            }
            html+='</td><td>';            
            html+=p.name.substr(0,32);
            html+='</td><td>';
            html+=(p.price+"CCC");
            html+='</td><td>';
            if(p.seller){
                if (p.seller.domain)
                    html+=p.seller.domain[0];
                else if (p.seller.id)
                html+=showID(p.seller.id);
            }
            html+='</td><td>';
            html+='<button class="btn btn-secondary pop" id="btn-'+p.id+' title="detail" onclick="{Shop.showDetails(\''+p.id+'\')}">Details</button>';
            html+='</td></tr>';
        }
        //console.log(html);
        $('#buy-product-list').html(html);
    }
    this.showDetails=function(id){
        console.log(id);            
        $("#modal-product-details").modal({backdrop: "static", show: true});           
        $("#modal-product-details").center();
        var p;
        for(var j in products)
            if(products[j].id==id){
                p=products[j];
                currentProduct=p;
            }
        
        if(p){
            var html="";
            html+="<div>Product ID:"+p.id+"</div>";            
            if (p.icon){
                html+='<div>'+CBrowser.createIconHtml(CBrowser.getB64DataFromLink(p.icon),p.icon,200,200)+'</div>';
            }            
            html+='<div>Product Name:'+p.name+'</div>';
            html+='<div>Price:'+(p.price+"CCC")+'</div>';
            if(p.shipmentfee)
                html+='<div>Shipment fee:'+(p.shipmentfee+"CCC")+'</div>';
            if(p.seller){
                if (p.seller.domain)
                    html+='<div>Shop:'+p.seller.domain[0]+'</div>';
                else if (p.seller.id)
                html+='<div>Shop:'+showID(p.seller.id)+'</div>';
            }
            if(p.intro)
                html+='<div><span>Introductions:</span>'+p.intro+'</div>';
            console.log(html);
            $("#modal-product-details").find(".modal-body").html(html);
        }
           
    }
    this.refreshCart=function(){        
        var hasContent=false;
        var html="<tr><th>Icon</th><th>Product Name</th><th>Seller</th><th>Price</th><th>Quantity</th><th>SubTotal</th><th></th></tr>";
        for(var j in cart){     
            hasContent=true;
            var p=cart[j];
            html+='<tr id="tr-'+p.link+'" title="'+p.link+'"><td>';
            if (p.icon){
                html+=CBrowser.createIconHtml(CBrowser.getB64DataFromLink(p.icon));
            }
            html+='</td><td>';            
            html+=p.name.substr(0,32);
            html+='</td><td>';            
            if(p.seller){
                if (p.seller.domain)
                    html+=p.seller.domain[0];
                else if (p.seller.id)
                html+=showID(p.seller.id);
            }
            html+='</td><td>';
            html+=(p.price+"CCC");
            html+='</td><td>';
            html+='<input name="quantity" type="text" style="width:40px" title="'+p.link+'" value="1" onkeyup="this.value=this.value.replace(/\D/g,\'\')" onafterpaste="this.value=this.value.replace(/\D/g,\'\')"/></td>';
            html+='<td><div class="subtotal" style="display:inline">'+p.price+'</div>&nbspCCC</td>';
            html+='<td><button class="btn btn-secondary" title="'+p.link+'">x</button>';
            html+='</td></tr>';
        }
        if(!hasContent)
            html="<tr><td>no products in cart.</td></tr>";
        console.log(html);
        $('#cart-list').html(html);
    }
    this.calCartTotal=function(){
        var total=0;
        $("#cart-list").each(function(){                
                var p=cart[$(this).attr("title")];
                var q=$(this).find("input").val();
                var subtotal=p.price*q;
                total+=subtotal;
                console.log(subtotal);
            });
        $("#cart-total").html(total);    
    };
    function bindReady() {
        if (haveBoundReady) {
            return;
        }
        haveBoundReady = true;
         //console.log("bindready");       
       
        $("#btn-add-new").unbind().click(function () {            
            var d = {};
            //d.domain = currentDomain;
            d.id=$("#add-new").find("input[name='product-id']").val().toString();
            var productname = $("#add-new").find("input[name='product-name']").val();            
            if(!productname){
                    i.makeNotice('error', 'publish-product-error', "Empty product name");
                    return 
            }
            d.name=productname;
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
            BrowserAPI.publishProduct(accountID, d, function (data) {
                    console.log(data);                    
                     i.makeNotice('success', 'publish-product-success', 'Product published!');
                }, function (e) {
                    i.makeNotice('error', 'publish-product-error', e);
                });            
        });
        $("#modal-product-details").find(".btn-secondary").unbind().click(function(){$("#modal-product-details").modal("hide");});
        $("#modal-product-details").find(".btn-primary").unbind().click(function(){
            $("#modal-product-details").modal("hide");
            for (var j in cart){
                if (j==currentProduct.link)
                    return;
            }
            cart[currentProduct.link]=currentProduct;            
            console.log(cart);
        });
        $("#cart-list tr").find("input[name='quantity']").unbind().bind("keyup change blur", function(){
            console.log("input value changed");
            if(isNaN($(this).val())||$(this).val()<0)
                $(this).val(1);                        
            $(this).next().html($(this).val()*cart[$(this).attr("title")].price);
            i.calCartTotal();
        });
        $("#btn-cart-buy").unbind().click(function () {             
            var l=[];
            $("#cart-list tr").each(function(){
                //console.log($(this));
                //console.log($(this).attr("title"));                
                var q={};
                var p=cart[$(this).attr("title")];
                if(p){
                    q.id=p.id;
                    q.link=p.link;
                    q.recipient=p.recipient;
                    q.price=p.price;
                    q.shipmengfee=p.shipmentfee;
                    q.quantity=parseInt($(this).find("input").val());   
                    console.log(q);
                    l.push(q);
                }
            });
            BrowserAPI.buyProducts(accountID,l);    
        });
        i.showViewProduct();
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