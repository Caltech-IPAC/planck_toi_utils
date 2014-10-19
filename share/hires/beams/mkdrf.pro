bmfile = '../../../../DR1/beam.v1'
dbread,'',bmfile,'STRING',dbbm,nc,nbms

img = mrdfits('DRF_857c_01.fits',0,hdr)
writefits,'DRF_857c_norm.fits',img/total(img),hdr
;hdr = headfits('DRF_857c_01.fits')

for i=0, nbms-1 do begin
   freq=reform(dbbm[0,i])
   fwhm = reform(dbbm[1,i])

   freqband = fix(freq)
   drf_id = fwhm+'x'+fwhm+'x2'

   ;npix = sxpar(hdr,'NAXIS1')
   
   extast,hdr,astr
   getrot,astr,rot,cdelt
   ;pixscl = mean(abs(cdelt))*60.
   if freqband LT 90. then pixscl = 2. else pixscl = 1.
   fwhm_pix = fwhm/pixscl

   npix =  round(fwhm/pixscl*2)
   ;print, i, freq, npix
   if npix mod 2 NE 1 then npix++
   ;print, npix

   sxaddpar,hdr,'NAXIS1',npix[0]
   sxaddpar,hdr,'NAXIS2',npix[0]
   sxaddpar,hdr,'CDELT1',-pixscl[0]/60.
   sxaddpar,hdr,'CDELT2',pixscl[0]/60.
   sxaddpar,hdr,'CRPIX1',(npix[0]-1)/2.+1
   sxaddpar,hdr,'CRPIX2',(npix[0]-1)/2.+1
   sxaddpar,hdr,'FREQBAND',freqband[0]
   sxaddpar,hdr,'DRF_ID',drf_id[0]
   psf = psf_Gaussian(Npixel=npix, fwhm=fwhm_pix, /normalize)

   outnm = 'DRF_'+freq+'.fits'
   writefits,outnm,psf,hdr
endfor

END
