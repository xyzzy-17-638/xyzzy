#ifndef _ARCHIVER_H_
# define _ARCHIVER_H_

# include "arc-if.h"

class ArchiverP
{
  const char *const *const ar_csuffixes;
  const char *const *const ar_esuffixes;
  const char *const *const ar_rsuffixes;

public:
  ArchiverP (const char *const *csuffixes,
             const char *const *esuffixes,
             const char *const *rsuffixes)
       : ar_csuffixes (csuffixes),
         ar_esuffixes (esuffixes), ar_rsuffixes (rsuffixes) {}
  static const char *const null_suffixes[];

  virtual int check_archive (const char *) const = 0;
  virtual int extract (HWND, const char *, const char *, const char *) const = 0;
  virtual   WORD get_version () const = 0;
  virtual   WORD get_sub_version () const = 0;
  virtual BOOL config_dialog (HWND hwnd, LPSTR buf, int mode) const = 0;
  virtual lisp list (const char *path, int file_name_only) const = 0;
  virtual void puts_extract (FILE *, char *) const = 0;

  virtual int create (HWND, const char *, const char *) const =0;
  virtual int create_sfx (HWND, const char *, const char *) const = 0;
  virtual void puts_create (FILE *, char *, const char *) const = 0;
  virtual int remove (HWND, const char *, const char *) const = 0;

  // default implementation
  int match_csuffix (const char *path, int l) const
    {return match_suffix (path, l, ar_csuffixes);}
  int match_esuffix (const char *path, int l) const
    {return match_suffix (path, l, ar_esuffixes);}
  int match_rsuffix (const char *path, int l) const
    {return match_suffix (path, l, ar_rsuffixes);}
  int match_suffix (const char *, int, const char *const *) const;
  virtual int reliable_checker_p () const {return 1;}
};

class UnzipBuiltin: public ArchiverP
{
  static const char *const esuffixes[];
public:
  UnzipBuiltin () : ArchiverP (null_suffixes, esuffixes, null_suffixes) {}
  virtual int check_archive (const char *) const ;
  virtual int extract (HWND, const char *, const char *, const char *) const ;
  virtual   WORD get_version () const ;
  virtual   WORD get_sub_version () const ;
  virtual BOOL config_dialog (HWND hwnd, LPSTR buf, int mode) const ;
  virtual lisp list (const char *path, int file_name_only) const ;
  virtual void puts_extract (FILE *, char *) const ;

  virtual int create (HWND, const char *, const char *) const {return ERROR_NOT_SUPPORT; }
  virtual int create_sfx (HWND, const char *, const char *) const {return ERROR_NOT_SUPPORT; }
  virtual void puts_create (FILE *, char *, const char *) const;
  virtual int remove (HWND, const char *, const char *) const {return ERROR_NOT_SUPPORT; }

};


class ArchiverPCommonArchiverImpl : public ArchiverP
{
public:
  const ArchiverInterface &ar_interface;
protected:
  ArchiverPCommonArchiverImpl (const ArchiverInterface &i,
             const char *const *csuffixes,
             const char *const *esuffixes,
             const char *const *rsuffixes)
       : ar_interface (i), ArchiverP(csuffixes, esuffixes, rsuffixes) {}
  int doit (HWND, const char *) const;
  int extract (HWND, const char *) const;
  int extract_noresp (HWND, const char *, int, const char *) const;
  int extract (HWND, const char *, const char *, const char *,
               const char *, const char *, const char *) const;
  int extract1 (HWND, const char *, const char *, const char *,
                const char *, const char *, const char *, char *) const;
  int create (HWND, const char *, const char *, const char *, const char *) const;
  int create1 (HWND, const char *, const char *, const char *, const char *, char *) const;
  int remove (HWND, const char *, const char *, const char *, const char *) const;
  int create_sfx (HWND, const char *, const char *, const char *) const;

  static void sepmap (char *, int, int);
  static void sepsl (char *path) {sepmap (path, '\\', '/');}
  static void sepbacksl (char *path) {sepmap (path, '/', '\\');}
public:
  virtual const char *match_any () const {return "";}
  virtual int check_archive (const char *) const;
  virtual int extract (HWND, const char *, const char *, const char *) const
    {return ERROR_NOT_SUPPORT;}
  virtual int create (HWND, const char *, const char *) const
    {return ERROR_NOT_SUPPORT;}
  virtual int create_sfx (HWND, const char *, const char *) const
    {return ERROR_NOT_SUPPORT;}
  virtual int remove (HWND, const char *, const char *) const
    {return ERROR_NOT_SUPPORT;}
  virtual void puts_create (FILE *, char *, const char *) const;
  virtual void puts_extract (FILE *, char *) const;
  virtual int post_open (HARC harc) const {return 0;}

  virtual   WORD get_version () const;
  virtual   WORD get_sub_version () const;
  virtual BOOL config_dialog (HWND hwnd, LPSTR buf, int mode) const;
  virtual lisp list (const char *path, int file_name_only) const;
};

class Ish: public ArchiverPCommonArchiverImpl
{
  IshInterface ish;
  static const char *const suffixes[];
public:
  Ish () : ArchiverPCommonArchiverImpl (ish, suffixes, suffixes, null_suffixes) {}
  virtual int extract (HWND, const char *, const char *, const char *) const;
  virtual int reliable_checker_p () const {return 0;}
};

class Tar: public ArchiverPCommonArchiverImpl
{
  TarInterface tar;
  static const char *const suffixes[];
public:
  Tar () : ArchiverPCommonArchiverImpl (tar, suffixes, suffixes, null_suffixes) {}
  virtual int extract (HWND, const char *, const char *, const char *) const;
  virtual int create (HWND, const char *, const char *) const;
  virtual void map_sl (char *path) const {sepsl (path);}
};

class Arj: public ArchiverPCommonArchiverImpl
{
  UnarjInterface arj;
  static const char *const suffixes[];
public:
  Arj () : ArchiverPCommonArchiverImpl (arj, suffixes, suffixes, null_suffixes) {}
  virtual int extract (HWND, const char *, const char *, const char *) const;
};

class Lha: public ArchiverPCommonArchiverImpl
{
  UnlhaInterface lha;
  static const char *const csuffixes[];
  static const char *const esuffixes[];
public:
  Lha () : ArchiverPCommonArchiverImpl (lha, csuffixes, esuffixes, csuffixes) {}
  virtual int extract (HWND, const char *, const char *, const char *) const;
  virtual int check_archive (const char *) const;
  virtual int create (HWND, const char *, const char *) const;
  virtual int remove (HWND, const char *, const char *) const;
  virtual void puts_extract (FILE *, char *) const;
  virtual int create_sfx (HWND, const char *, const char *) const;
  virtual const char *match_any () const {return "*.*";}
};


class Unzip: public ArchiverPCommonArchiverImpl
{
  UnzipInterface unzip;
  static const char *const esuffixes[];
public:
  Unzip () : ArchiverPCommonArchiverImpl (unzip, null_suffixes, esuffixes, null_suffixes) {}
  virtual int extract (HWND, const char *, const char *, const char *) const;
  virtual void map_sl (char *path) const {sepsl (path);}
  virtual void puts_extract (FILE *, char *) const;
};


class Zip: public ArchiverPCommonArchiverImpl
{
  ZipInterface zip;
  const Unzip &unzip;
  static const char *const csuffixes[];
public:
  Zip (const Unzip &unzip_)
       : ArchiverPCommonArchiverImpl (zip, csuffixes, null_suffixes, csuffixes), unzip (unzip_) {}
  virtual int create (HWND, const char *, const char *) const;
  virtual int remove (HWND, const char *, const char *) const;
  virtual int check_archive (const char *) const;
  virtual void puts_create (FILE *, char *, const char *) const;
};

class Cab: public ArchiverPCommonArchiverImpl
{
  CabInterface cab;
  static const char *const csuffixes[];
  static const char *const esuffixes[];
public:
  Cab () : ArchiverPCommonArchiverImpl (cab, csuffixes, esuffixes, null_suffixes) {}
  virtual int extract (HWND, const char *, const char *, const char *) const;
  virtual int create (HWND, const char *, const char *) const;
};

class Unrar: public ArchiverPCommonArchiverImpl
{
  UnrarInterface unrar;
  static const char *const esuffixes[];
public:
  Unrar () : ArchiverPCommonArchiverImpl (unrar, null_suffixes, esuffixes, null_suffixes) {}
  virtual int extract (HWND, const char *, const char *, const char *) const;
};

class Bga: public ArchiverPCommonArchiverImpl
{
  BgaInterface bga;
  static const char *const suffixes[];
public:
  Bga () : ArchiverPCommonArchiverImpl (bga, suffixes, suffixes, suffixes) {}
  virtual int extract (HWND, const char *, const char *, const char *) const;
  virtual int create (HWND, const char *, const char *) const;
  virtual int remove (HWND, const char *, const char *) const;
  virtual const char *match_any () const {return "*.*";}
};

class Yz1: public ArchiverPCommonArchiverImpl
{
  Yz1Interface yz1;
  static const char *const suffixes[];
public:
  Yz1 () : ArchiverPCommonArchiverImpl (yz1, suffixes, suffixes, null_suffixes) {}
  virtual int extract (HWND, const char *, const char *, const char *) const;
  virtual int create (HWND, const char *, const char *) const;
  virtual int post_open (HARC harc) const
    {return yz1.set_default_passwd (harc, 0);}
};

class UnGCA: public ArchiverPCommonArchiverImpl
{
  UnGCAInterface ungca;
  static const char *const esuffixes[];
public:
  UnGCA () : ArchiverPCommonArchiverImpl (ungca, null_suffixes, esuffixes, null_suffixes) {}
  virtual int extract (HWND, const char *, const char *, const char *) const;
};

class SevenZip: public ArchiverPCommonArchiverImpl
{
  SevenZipInterface seven_zip;
  static const char *const suffixes[];
public:
  SevenZip () : ArchiverPCommonArchiverImpl (seven_zip, suffixes, suffixes, suffixes) {}
  virtual int extract (HWND, const char *, const char *, const char *) const;
  virtual int create (HWND, const char *, const char *) const;
  virtual int remove (HWND, const char *, const char *) const;
  virtual void puts_create (FILE *, char *, const char *) const;
};

class Archiver
{
protected:
  Ish a_ish;
  Tar a_tar;
  Arj a_arj;
  Lha a_lha;
  Unzip a_unzip;
  UnzipBuiltin a_unzip_bulitin;
  Zip a_zip;
  Cab a_cab;
  Unrar a_unrar;
  Bga a_bga;
  Yz1 a_yz1;
  UnGCA a_ungca;
  SevenZip a_seven_zip;
  enum {NARCS = 13};
  ArchiverP *arcs[NARCS];

  static int check_file_size (const char *);
public:
  Archiver ();
  const ArchiverP *get_creator (const char *) const;
  const ArchiverP *get_extractor (const char *) const;
  const ArchiverP *get_remover (const char *) const;
  const ArchiverP *check_archive (const char *) const;
  int extract (HWND, const char *, const char *, const char *) const;
  int create (HWND, const char *, const char *) const;
  int create_sfx (HWND, const char *, const char *) const;
  lisp list (const char *, int) const;
  static int get_version (const ArchiverP &, char *);
  static int config_dialog (const ArchiverP &, HWND hwnd, int mode);
  const ArchiverP *get (lisp) const;
};

#endif
