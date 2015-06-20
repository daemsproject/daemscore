/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
var Messenger = new function() {
    var i = this;
    var tx_page;
    this.skip_init = false; //Set on sign up page
    var cVisible; //currently visible view    
    var accountID;    
    var transactions = []; //List of all transactions (initially populated from /multiaddr updated through websockets)    
    var contacts=[];
    var archTimer; //Delayed Backup wallet timer
    var currentContact;
    var event_listeners = []; //Emits Did decrypt wallet event (used on claim page)
    var messages=[];
    var isInitialized = false;    
    var language = 'en'; //Current language    
    var haveBoundReady = false;
    
    var sync_pubkeys = false;    
    var wallet_options = {    
        fee_policy : 0,  //Default Fee policy (-1 Tight, 0 Normal, 1 High)
        html5_notifications : false, //HTML 5 Desktop notifications    
        tx_display : 0, //Compact or detailed transactions    
        transactions_per_page : 1000, //Number of transactions per page    
    };
    this.setNTransactionsPerPage = function(val) {
        wallet_options.transactions_per_page = val;
    }

    this.getNTransactionsPerPage = function() {
        return wallet_options.transactions_per_page;
    }
    function bindInitial() {       

        $('.modal').on('show', function() {
            hidePopovers();

            $(this).center();
        }).on('hidden', function () {
                var visible = $('.modal:visible');

                var notices = $('#notices').remove();

                if (visible.length > 0)
                    visible.find('.modal-body').prepend(notices);
                else
                    $('#main-notices-container').append(notices);

            }).on('shown', function() {
                hidePopovers();

                var self = $(this);
                setTimeout(function() {
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
        } catch (e) {}
    }
    this.makeNotice = function(type, id, msg, timeout) {

        if (msg == null || msg.length == 0)
            return;

        console.log(msg);

        var el = $('<div class="alert alert-block alert-'+type+'"></div>');

        el.text(''+msg);

        if ($('#'+id).length > 0) {
            el.attr('id', id);
            return;
        }

        $("#notices").append(el).hide().fadeIn(200);

        (function() {
            var tel = el;

            setTimeout(function() {
                tel.fadeOut(250, function() {
                    $(this).remove();
                });
            }, timeout ? timeout : 5000);
        })();
    }
    this.switchToContact=function(id){
        updateCurrentContactDisplay(id);
        //$("#current-contact").html(id);
        $('#'+currentContact).attr('class', '');
        $('#'+id).attr('class', 'active');
        currentContact=id;
        $("#btn-currentcontact").show();
        i.showMessages(id);
    }
    function updateCurrentContactDisplay(id){        
        var c=contacts[id];  
        var html="";
        if (c.image)            
            html+=createImgHtml(c.image);        
        if(c.alias)
        html+=c.alias+'<br>';
        if(c.domainName)
            html+=c.domainName+'<br>';
        html+=id; 
        $("#current-contact").html(html);
    }
    function addContact(id){        
        if(contacts[id])
            return;
        contacts[id]={};
        contacts[id]=i.readContactInfo(id);
        //contacts[id].id=id;            
        var html='<div id="'+id+'" onclick="{Messenger.switchToContact(\''+id+'\')}" style="margin-top:20px"></div';
        $("#contact-list").append(html);
        i.updateContactDisplay(id);
    }    
    this.updateContactDisplay=function(id){
        if(!contacts[id])
            return;
        var c=contacts[id];  
        var html="";
        if (c.image)            
            html+=createImgHtml(c.image);
        //c.alias=i.getAlias(c.id);
        if(c.alias)
        html+='<div style="float:left">'+c.alias+'&nbsp  </div>';
        else
        html+=" "+showID(id);
        html+="<div class='nmsgs' style='text-align:right'></div>";        
        $("#"+id).html(html);
        i.updateUnreadLen(id);
        if(currentContact==id){
            updateCurrentContactDisplay(id);
        }
    }
    this.showMessage=function(msg,time,mode,direction){    
        if(!time)
            time= new Date();
        var html='<div style="color:green;text-align:'+direction+'">'+dateToString(time)+" "+mode+'</div>';        
        html+='<div style="text-align:'+direction+';margin-'+(direction=='right'?'left':'right')+':100px">';
        if(direction=="left")
            html+="->";
        else
            html+="<-";
                html+=msg+'</div>';
        //console.log(html);
        $('#history-message').append(html);
        $('#history-message').scrollTop($('#history-message').scrollTop()+$('#history-message').innerHeight());
    }
    this.showMessages=function(id){
        
        $('#history-message').html(" ");
//        var msgs=[];
//        for(var j in messages[id]){
//            msgs.push(messages[id][j].content);
//        }
//        console.log(msgs);
        i.decryptAndShow(id,messages[id]);
        
    }    
    this.decryptAndShow=function(id,msgs){
        
        var msgs2=[];
        for(var j in msgs){
//            if (Object.prototype.toString.call(msgs[j].content) != '[object Array]')   {
//                console.log(j);
//                console.log(msgs[j]);
//            }
            msgs2.push(msgs[j].content);
        }
        //console.log(msgs2);
        BrowserAPI.decryptMessages(accountID,[{idForeign:id,messages:msgs2}],function(decryptmsgs){
            //console.log(decryptmsgs);        
            for(var j=decryptmsgs[0].messages.length-1;j>=0;j--){
                if(decryptmsgs[0].messages[j]){
                    //console.log(messages[id][j]);
                    //console.log(id);
                    //console.log(BrowserAPI.areIDsEqual(messages[id][j].IDTo,id));
                    var direction=BrowserAPI.areIDsEqual(msgs[j].IDTo,id)?"right":"left";   
                    if(decryptmsgs[0].messages[j][0]){
                        msgs[j].decrypted=base64.decode(decryptmsgs[0].messages[j][0].content[0].content);                        
                        i.updateMessage(msgs[j]);
                        //console.log(msgs[j]);
                        //var msg=base64.decode(decryptmsgs[0].messages[j][0].content[0].content);
                        var t=new Date(msgs[j].nTime * 1000);
                        i.showMessage(msgs[j].decrypted,t,msgs[j].mode,direction);
                    }
                }
            } 
            i.setLastUpdateTime(id);
            i.updateUnreadLen(id);
        },function(e){console.log(e)});
    }
    this.updateMessage=function(msg){
        if(msg.IDForeign){
            for( var j in messages[msg.IDForeign]){
                var msg2=messages[msg.IDForeign][j];
                if(msg2.txid==msg.txid&&msg2.nVout==msg.nVout)
                    msg2.decrypted=msg.decrypted;
            }
        }
    }
    this.addMessage=function(msg){         
        msg.IDForeign=BrowserAPI.areIDsEqual(msg.IDFrom,accountID)?msg.IDTo:msg.IDFrom;
        if(i.hasMessage(msg))
            return;
        addContact(msg.IDForeign);
        if(!messages[msg.IDForeign]){
            messages[msg.IDForeign]=[];
            //messages[msg.IDForeign].updateTime=i.getLastUpdateTime(msg.IDForeign);
        }
        messages[msg.IDForeign].push(msg);
        //msg.unreadLen=i.getUnreadLen(msg.IDForeign);
        //if(msg.unreadLen)
            i.updateUnreadLen(msg.IDForeign);
    }
    this.getLastUpdateTime=function(id){
        var t=BrowserAPI.getConf("messenger",accountID,id,"updatetime");  
        if(!t||isNaN(t))
            return 0;
        return t;
    }
    this.setLastUpdateTime=function(id){
        return BrowserAPI.setConf("messenger",accountID,id,"updatetime",String(new Date().getTime()));        
    }
    this.updateUnreadLen=function(id){    
        var a=i.getUnreadLen(id);
        if(!a)
            a=".";
         $("#"+id).find(".nmsgs").html(a);
    }
    this.getUnreadLen=function(id){
        if(!messages[id])
            return 0;                            
        var t0=i.getLastUpdateTime(id);
        //console.log(t0);
        var n=0;
        for(var j in messages[id]){
            //var t=new Date(messages[id][j].nTime * 1000);
            if(messages[id][j].nTime*1000>t0)
                n++;
        }
        //console.log(n);
        return n;
    }
    this.getAlias=function(id){
        var alias=BrowserAPI.getConf("messenger",accountID,id,"alias");   
        if(alias.error)
            alias="";
        return alias;
    }
    this.setAlias=function(id,alias){
        var result=BrowserAPI.setConf("messenger",accountID,id,"alias",alias);   
        if(result=="success")
            return true;
        return false;
    }
    this.getCategory=function(id){
        var category=BrowserAPI.getConf("messenger",accountID,id,"category");   
        if(category.error)
            category="";
        return category;
    }
    this.setCategory=function(id,category){
        var result=BrowserAPI.setConf("messenger",accountID,id,"category",category);   
        if(result=="success")
            return true;
        return false;
    }
    this.readContactInfo=function(id){
        var c={};
        c.alias=i.getAlias(id);
        c.updateTime=i.getLastUpdateTime(id);
        c.category=i.getCategory(id);        
        return c;
    }
    this.getMessagesAll=function(){
        console.log("getMessages");      
        
        BrowserAPI.getMessages(accountID,null,0,true,false,0,1000,function(data){
            if (!data||data.error){
                if (error) error();
                return;                
            }
            console.log(data); 
                for(var l in data){                    
                    data[l].mode="onchain";
                    i.addMessage(data[l]);
                }
        }, function() {
            console.log("getMessage error"); 
        });
    };

    function registerNotifications(){
        //var aa=fuction(a){(a);};
//        var aa=function(a){
//            MyWallet.notifiedBlock(a);
//        };
        console.log("regnotifications");
        var ab=function(a){
            Messenger.notifiedTx(a);
        };
        var ac=function(a){
            console.log("refresh");
            window.location.href=window.location.href;
        }
        //BrowserAPI.regNotifyBlocks(aa);        
        BrowserAPI.regNotifyTxs(ab,[accountID]);
        BrowserAPI.regNotifyAccount(ac);
        console.log("regnotifications success");
//        BrowserAPI.regNotifyPeers(this.notifiedPeers);
    }
    this.hasMessage=function(msg){
        if(!messages[msg.IDForeign])
            return false;
        for(var j in messages[msg.IDForeign]){
            if (messages[msg.IDForeign][j].txid==msg.txid&&messages[msg.IDForeign][j].nVout==msg.nVout)
                return true;
        }
            return false;
    }
    this.notifiedTx=function(x){
        console.log(x);
        var data=BrowserAPI.getTxMessages(accountID,[x.tx.txid]);     
        console.log(data);
        if (data||!data.error){
            var msgs=[];
            for(var j in data){   
                msg=data[j];                
                msgs.push(msg);
               Messenger.addMessage(msg);        
            }
            //console.log(msg.IDFrom);
            //console.log(currentContact);
            //console.log(msg.IDTo);
            //console.log(accountID);           
            if(BrowserAPI.areIDsEqual(msg.IDFrom,currentContact)&&BrowserAPI.areIDsEqual(msg.IDTo,accountID)){
               // console.log(JSON.stringify(data));
                   i.decryptAndShow(currentContact,data);
           }
        }               
    };
    
    this.notifiedPeers=function(data){
        
    }
   
    //Reset is true when called manually with changeview
    function buildVisibleViewPre() {
        //Hide any popovers as they can get stuck whent the element is re-drawn
        hidePopovers();

        //Update the account balance
        if (balance == null) {
            //$('#balance').html('Loading...');
        } else {
            //$('#balance').html(formatSymbol(final_balance, symbol, true));
            //$('#balance2').html(formatSymbol(final_balance, (symbol === symbol_local) ? symbol_btc : symbol_local), true);
        }

        //Only build when visible
        return cVisible.attr('id');
    }
    function changeCategory(id) {
        //if (id === cVisible)
        //    return;

        if (cVisible != null) {
            if ($('#' + cVisible.attr('id') ).length > 0)
                $('#' + cVisible.attr('id') ).parent().attr('class', '');

           // cVisible.hide();
        }
        cVisible = id;
        //cVisible.show();
        if ($('#' + cVisible.attr('id') ).length > 0)
            $('#' + cVisible.attr('id') ).parent().attr('class', 'active');        
        switch (id.attr('id')){
            case "list-all":        
                for(var j in contacts){   
                    $("#"+j).hide();
                     if(contacts[j].category!="black")
                    $("#"+j).show();
                }
            break;
        case "list-friends":
                for(var j in contacts){                    
                    $("#"+j).hide();
                    if(contacts[j].category=="friend")
                        $("#"+j).show();
                }       
            break;
        case "list-black":
                for(var j in contacts){                    
                    $("#"+j).hide();
                    if(contacts[j].category=="black")
                        $("#"+j).show();
                }       
            break;
        }
    
    }
    function bindReady() {
        if (haveBoundReady) {
            return;
        }
        
        haveBoundReady = true;
        $("#list-all").click(function() {
            changeCategory($("#list-all"));
        });
        $("#list-friends").click(function() {
            changeCategory($("#list-friends"));
        });
        $("#list-black").click(function() {
            changeCategory($("#list-black"));
        });
        $("#add-contact").find(".btn-secondary").unbind().click(function() {               
            $("#add-contact").modal("hide");
       });
        $("#add-contact").find(".btn-primary").unbind().click(function() { 
            var id=$("#add-contact").find('input[name="contact-id"]').val();
            if(contacts[id]){
                Messenger.makeNotice('error', 'error', 'ID already in contact list');    
                return;
            }            
            if(!BrowserAPI.checkNameKey(id)){
                 Messenger.makeNotice('error', 'error', 'ID is not valid');                 
                return;
            }            
            addContact(id);            
            //var arr=[{id:$("#add-contact").find('input[name="contact-id"]').val()}];            
//            BrowserAPI.add_contacts(accountID,arr,function(a){
//            },function(e){
//                Messenger.makeNotice('error', 'add-contact-error',e);
//            })
            $("#add-contact").modal("hide");
       });    
        
        
        $('#btn-send').unbind().click(function() {   
            //console.log($("#send-message-box").val());
            var msg=$("#send-message-box").val();
            if(currentContact&&msg){                
                BrowserAPI.sendMessage(accountID,currentContact,msg,function(){
                    i.showMessage(msg,0,"right");
                    i.setLastUpdateTime(currentContact);
                    $("#send-message-box").val("");
                    
                },function(e){
                    i.makeNotice('error', 'send-message-error', e);
           
                });
                    
            }
       });
       $('#btn-add').unbind().click(function() { 
           $("#add-contact").modal({backdrop: "static", show: true});
           //$("#add-contact").show();
           $("#add-contact").center();
           
       });    
       $("#btn-currentcontact").unbind().click(function() { 
           $("#edit-modal-contact").html(currentContact);
           $("#edit-contact").find("input[name='contact-alias']").val(contacts[currentContact].alias);           
           var cc=contacts[currentContact].category;
           if(!cc)
               cc="none";
           $('#contact-category').html(cc);
           switch(cc){
               case "none":
                $("#btn-friend").text("Add To Friends");
                 $("#btn-black").text("Add To blacklist");
                 break;
             case "friend":
                 $("#btn-friend").text("Add To blacklist");
                 $("#btn-black").text("Remove from Friends");
                 break;
             case "black":
                 $("#btn-friend").text("Add To Friends");
                 $("#btn-black").text("Remove from blacklist");
           }
           $("#edit-contact").modal({backdrop: "static", show: true});
           $("#edit-contact").center();
       });
        $("#edit-contact").find(".btn-secondary").unbind().click(function() {               
            $("#edit-contact").modal("hide");
       });
        $("#btn-friend").unbind().click(function() {              
            if(contacts[currentContact].category!="friend"){
                contacts[currentContact].category="friend";
                i.setCategory(currentContact,"friend");
                $('#contact-category').html("friend");
                $("#btn-friend").text("Remove from Friends");
                $("#btn-black").text("Add To blacklist");
            }else{
                contacts[currentContact].category="none";
                i.setCategory(currentContact,"none");
                $('#contact-category').html("none");
                $("#btn-friend").text("Add To Friends");
                $("#btn-black").text("Add To blacklist");
            }
            changeCategory(cVisible);
       });
        $("#btn-black").unbind().click(function() {               
            if(contacts[currentContact].category!="black"){
                contacts[currentContact].category="black";
                i.setCategory(currentContact,"black");
                $('#contact-category').html("black");
                $("#btn-black").text("Remove from blacklist");
                $("#btn-friend").text("Add To Friends");
            }else{
                contacts[currentContact].category="none";
                i.setCategory(currentContact,"none");
                $('#contact-category').html("none");
                $("#btn-black").text("Add To blacklist");
                $("#btn-friend").text("Add To Friends");
            }
            changeCategory(cVisible);
       });
       $("#btn-alias").unbind().click(function() { 
           //console.log($("#edit-contact").find("input[name='contact-alias']").val());
           var alias=$("#edit-contact").find("input[name='contact-alias']").val();
           i.setAlias(currentContact,alias);
           contacts[currentContact].alias=alias;
           i.updateContactDisplay(currentContact);
       });
    }

    function initAccount(){
        accountID=BrowserAPI.getAccountID();          
        $("#account-id").html(accountID);
        //IDs=BrowserAPI.getIDs(accountID);  
        //BrowserAPI.getNewID(accountID);
        registerNotifications();
        //readContacts(Messenger.get_history);    
        Messenger.getMessagesAll();
    }
    
    
   $(document).ready(function() {

        if (!$.isEmptyObject({}) || !$.isEmptyObject([])) {
            Messenger.makeNotice('error', 'error', 'Object.prototype has been extended by a browser extension. Please disable this extensions and reload the page.');
            return;
        }

        //Listener to reload the page on App Cache update
        window.applicationCache.addEventListener('updateready', function () {
            if(window.applicationCache.status === window.applicationCache.UPDATEREADY) {
                window.applicationCache.swapCache();
                location.reload();
            }
        });

        //Disable autocomplete in firefox
        $("input,button,select").attr("autocomplete","off");
        var body = $(document.body);

        
         
        bindInitial();  
        initAccount(); 
        bindReady();
        changeCategory($("#list-all"));
        //Frame break
        if (top.location != self.location) {
            top.location = self.location.href
        }

       

        

        $(document).ajaxStart(function() {
                setLogoutImageStatus('loading_start');

                $('.loading-indicator').fadeIn(200);
            }).ajaxStop(function() {
                setLogoutImageStatus('loading_stop');

                $('.loading-indicator').hide();
            });
    });
}