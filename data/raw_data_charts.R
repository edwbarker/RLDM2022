rm(list=ls())

# CODE FOR RLDM SUBMISSION

library(simpleboot)
library(boot)

power<-4 # parameter for bootstrapped confidence intervals (10^power)
project<-'D1'
trials<-50

most_recent<-function(...) {
  tail(list.files(...), 1)
}

# edit to reflect folder storing raw data
setwd('~/RLDM2022/data/')

# import results from C++ (compound)

data_PE_0<-read.csv(most_recent(pattern=paste0(project,'_PE_V0.csv')), header=F, stringsAsFactors=F)
data_PE_10<-read.csv(most_recent(pattern=paste0(project,'_PE_V10.csv')), header=F, stringsAsFactors=F)
data_PE_25<-read.csv(most_recent(pattern=paste0(project,'_PE_V25.csv')), header=F, stringsAsFactors=F)
data_PE_50<-read.csv(most_recent(pattern=paste0(project,'_PE_V50.csv')), header=F, stringsAsFactors=F)
data_PE_100<-read.csv(most_recent(pattern=paste0(project,'_PE_V100.csv')), header=F, stringsAsFactors=F)

data_PI_0<-read.csv(most_recent(pattern=paste0(project,'_PI_V0.csv')), header=F, stringsAsFactors=F)
data_PI_10<-read.csv(most_recent(pattern=paste0(project,'_PI_V10.csv')), header=F, stringsAsFactors=F)
data_PI_25<-read.csv(most_recent(pattern=paste0(project,'_PI_V25.csv')), header=F, stringsAsFactors=F)
data_PI_50<-read.csv(most_recent(pattern=paste0(project,'_PI_V50.csv')), header=F, stringsAsFactors=F)
data_PI_100<-read.csv(most_recent(pattern=paste0(project,'_PI_V100.csv')), header=F, stringsAsFactors=F)

range<-(trials + 3):(trials * 2 + 2)
range<-range[-49] # removal of unexplained outlier

data_PE_0<-abs(as.matrix(data_PE_0[range,c(-1,-102)]))
data_PE_10<-abs(as.matrix(data_PE_10[range,c(-1,-102)]))
data_PE_25<-abs(as.matrix(data_PE_25[range,c(-1,-102)]))
data_PE_50<-abs(as.matrix(data_PE_50[range,c(-1,-102)]))
data_PE_100<-abs(as.matrix(data_PE_100[range,c(-1,-102)]))

data_PE_0_av<-apply(data_PE_0,2,mean)
data_PE_10_av<-apply(data_PE_10,2,mean)
data_PE_25_av<-apply(data_PE_25,2,mean)
data_PE_50_av<-apply(data_PE_50,2,mean)
data_PE_100_av<-apply(data_PE_100,2,mean)

range<-2:(trials + 1)

data_PI_0<-as.matrix(data_PI_0[range,c(-1,-102)])
data_PI_10<-as.matrix(data_PI_10[range,c(-1,-102)])
data_PI_25<-as.matrix(data_PI_25[range,c(-1,-102)])
data_PI_50<-as.matrix(data_PI_50[range,c(-1,-102)])
data_PI_100<-as.matrix(data_PI_100[range,c(-1,-102)])

data_PI_0_av<-apply(data_PI_0,2,mean)
data_PI_10_av<-apply(data_PI_10,2,mean)
data_PI_25_av<-apply(data_PI_25,2,mean)
data_PI_50_av<-apply(data_PI_50,2,mean)
data_PI_100_av<-apply(data_PI_100,2,mean)


# confidence intervals

upper_vec_list<-list()
lower_vec_list<-list()

for (data in list(data_PE_0, data_PE_100, data_PI_0, data_PI_100)) {

  upper_vec<-NULL
  lower_vec<-NULL
  
  for (i in 1:ncol(data)) {
    
    cat(paste0(" ... ",i)) # tracking, can take some time
    
    # bootstrap confidence interval for this iteration
    x.boot<-one.boot(data[,i], mean, R=10^power)
    # confidence interval
    tryCatch({
      ci.boot<-boot.ci(x.boot, type="bca", conf=0.68)
      upper<-ci.boot$bca[[5]]
      lower<-ci.boot$bca[[4]]
    }, error= function(e) {
      ci.boot<-apply(data,2,mean)
      upper<-ci.boot
      lower<-ci.boot
    })
    
    # add to vector for all iterations
    upper_vec<-c(upper_vec,upper)
    lower_vec<-c(lower_vec,lower)
    
  }
  
  upper_vec_list<-c(upper_vec_list, list(upper_vec))
  lower_vec_list<-c(lower_vec_list, list(lower_vec))

}

# create charts image

# edit location to save plots
pdf(file="~/results.pdf",width=8, height=1.48)

layout_matrix<-matrix(c(1,2,3),nrow=1,ncol=3)
layout(layout_matrix, widths=c(3,3,1))

par(mar=c(2,4,0.2,2))

plot(data_PE_0_av, ylim=c(0,max(data_PE_0_av,data_PE_10_av,data_PE_25_av,data_PE_50_av,data_PE_100_av)), type="l", 
     lwd=3, xlab="Iterations", ylab="Mean sq. VF error", col="white")
mtext(expression(italic(t)~x*10^4), side=1, line=1.03, at=-11, cex=0.7)
polygon(c(1:100,100:1), c(upper_vec_list[[1]], lower_vec_list[[1]][100:1]), col=rgb(234/255, 206/255, 9/255, 0.15), border=NA)
polygon(c(1:100,100:1), c(upper_vec_list[[2]], lower_vec_list[[2]][100:1]), col=rgb(0,0,1,0.1), border=NA)
lines(data_PE_0_av, col=rgb(234/255, 206/255, 9/255), lty=1, lwd=3)
lines(data_PE_10_av, col="forestgreen", lty=4, lwd=3)
lines(data_PE_25_av, col="purple", lty=3, lwd=3)
lines(data_PE_50_av, col="red", lty=5, lwd=3)
lines(data_PE_100_av, col="blue", lty=2, lwd=3)

plot(data_PI_0_av, ylim=c(0,max(data_PI_0_av,data_PI_10_av,data_PI_25_av,data_PI_50_av,data_PI_100_av)), type="l", 
     lwd=3, xlab="Iterations", ylab="Average reward", col="white")
mtext(expression(italic(t)~x~10^4), side=1, line=1.03, at=-11, cex=0.7)
polygon(c(1:100,100:1), c(upper_vec_list[[3]], lower_vec_list[[3]][100:1]), col=rgb(234/255, 206/255, 9/255, 0.15), border=NA)
polygon(c(1:100,100:1), c(upper_vec_list[[4]], lower_vec_list[[4]][100:1]), col=rgb(0,0,1,0.1), border=NA)
lines(data_PI_0_av, col=rgb(234/255, 206/255, 9/255), lty=1, lwd=3)
lines(data_PI_10_av, col="forestgreen", lty=4, lwd=3)
lines(data_PI_25_av, col="purple", lty=3, lwd=3)
lines(data_PI_50_av, col="red", lty=5, lwd=3)
lines(data_PI_100_av, col="blue", lty=2, lwd=3)

par(mar=c(0,0,0,0))

text_size<-1

plot.new()
di<-par("usr")
legend(xjust=0.5,yjust=0.45,ncol=1,x=di[1]+0.5*(di[2]-di[1]),y=di[3]+0.5*(di[4]-di[3]), bty="n", cex=text_size, 
       legend=c("V = 0", "V = 10", "V = 25", "V = 50", "V = 100"), col=c(rgb(234/255, 206/255, 9/255),"forestgreen","purple","red","blue"),
       lwd=3,lty=c(1,4,3,5,2))
legend(xjust=0.5,yjust=0.45,ncol=1,x=di[1]+0.5*(di[2]-di[1]),y=di[3]+0.5*(di[4]-di[3]), bty="n", cex=text_size, 
       legend=c("             ","","","",""), col=c(rgb(234/255, 206/255, 9/255, 0.2),rgb(0,0,0,0),rgb(0,0,0,0),rgb(0,0,0,0),rgb(0,0,1,0.15)),lwd=8)

dev.off()